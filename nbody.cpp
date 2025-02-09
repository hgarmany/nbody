#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include "physics.h"
#include "controls.h"
#include "builder.h"

#include "stb_image.h"

std::mutex physicsMutex;
std::condition_variable physicsCV;
bool physicsUpdated = false;
std::atomic<bool> running(true);

Shader shader, skyboxShader, trailShader;
glm::mat4 projection;
GLFWwindow* window;

GLuint trailVAO, trailVBO, trailAlphaBuf;
int physicsFrames, lastPhysicsFrames = 0;
size_t cube, sphere;
int screenSize = WIDTH;

size_t maxTrailLength = 2500;
double initTime;

// Function to update the projection matrix based on window size
void static updateProjectionMatrix(GLFWwindow* window) {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);  // Get the current framebuffer size

	if (height == 0) height = 1;  // Prevent division by zero

	// Calculate the aspect ratio dynamically
	float aspect = (float)width / (float)height;

	// Define the projection matrix (FOV, aspect ratio, near, far)
	projection = glm::perspective(FOV * height / screenSize, aspect, 1e0f, 1e9f);
}

// Set this function as a callback to update projection matrix during window resizing
void static window_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);  // Set the OpenGL viewport to match the new window size
	updateProjectionMatrix(window);  // Update the projection matrix with the new size
}

void static initWindow() {
	if (!glfwInit()) {
		std::cerr << "GLFW initialization failed!" << std::endl;
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_DEPTH_BITS, 32);

	// multisample buffer for antialiasing
	glfwWindowHint(GLFW_SAMPLES, 4);

	window = glfwCreateWindow(WIDTH, HEIGHT, "N-Body Simulator", nullptr, nullptr);

	if (!window) {
		std::cerr << "Window creation failed!" << std::endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int width, int height) {
		glViewport(0, 0, width, height);
		});

	glewInit();

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback); // Set scroll callback
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetKeyCallback(window, key_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide and capture cursor initially
}

void setPV(Shader& shader, glm::mat4& P, glm::mat4& V) {
	glUniformMatrix4fv(shader.P, 1, GL_FALSE, &P[0][0]);
	glUniformMatrix4fv(shader.V, 1, GL_FALSE, &V[0][0]);
}

void static render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Activate the shader program
	glUseProgram(shader.index);
	glm::vec3 lightPos = bodies[0].position;
	glm::vec3 lightColor = glm::vec3(1.0f);
	glUniform3fv(shader.uniforms[LIGHT_POS], 1, &lightPos[0]);
	glUniform3fv(shader.uniforms[LIGHT_COLOR], 1, &lightColor[0]);

	// set camera
	glm::mat4 view = camera.viewMatrix();
	setPV(shader, projection, view);

	Camera copy = camera;
	for (Entity body : bodies) {
		copy.position = camera.position - body.position;
		glm::mat4 relativeView = copy.viewMatrix();
		glUniform3fv(shader.uniforms[LIGHT_POS], 1, &(lightPos - glm::vec3(body.position))[0]);
		setPV(shader, projection, relativeView);
		glm::dvec3 bodyPos = body.position;
		body.position = glm::dvec3(0.0);
		body.draw(shader, MODE_TEX);
		body.position = bodyPos;
	}

	// start trails

	glUseProgram(trailShader.index);
	setPV(trailShader, projection, view);
	std::vector<glm::vec3> trailVertices;
	std::vector<float> trailAlphas;
	for (GravityBody body : bodies) {
		if (body.trail) {
			trailVertices.insert(trailVertices.end(), body.trail->begin(), body.trail->end());
			for (size_t j = 0; j < body.trail->size(); ++j) {
				trailAlphas.push_back(static_cast<float>(j) / body.trail->size());
			}
		}
	}

	glBindVertexArray(trailVAO);
	glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
	glBufferData(GL_ARRAY_BUFFER, trailVertices.size() * sizeof(glm::vec3), trailVertices.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0); // Assuming location 0 for positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

	glBindBuffer(GL_ARRAY_BUFFER, trailAlphaBuf);
	glBufferData(GL_ARRAY_BUFFER, trailAlphas.size() * sizeof(float), trailAlphas.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(1); // Assuming location 1 for alphas
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), 0);

	GLint offset = 0;
	glm::vec3 root(0.0);
	for (GravityBody body : bodies) {
		if (body.trail) {
			glUniform3fv(trailShader.uniforms[OBJ_COLOR], 1, &body.trailColor[0]);
			if (body.parentIndex != -1)
				root = glm::vec3(bodies[body.parentIndex].position);
			else
				root = glm::vec3(0.0);
			glUniform3fv(trailShader.uniforms[OBJ_POS], 1, &root[0]);
			glDrawArrays(GL_LINE_STRIP, offset, (GLsizei)body.trail->size());
			offset += (GLint)body.trail->size();
		}
	}

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glBindVertexArray(0); // Unbind VAO

	// end trails
	
	glDepthFunc(GL_LEQUAL);
	glUseProgram(skyboxShader.index);

	// set camera
	view = glm::mat4(glm::mat3(view));
	setPV(skyboxShader, projection, view);

	Entity::skybox.draw(skyboxShader, MODE_CUBEMAP);

	glDepthFunc(GL_LESS);
}

void static MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
		fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
			(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
			type, severity, message);
}

void updateTrails(double time) {
	for (GravityBody body : bodies) {
		auto trail = body.trail;

		if (trail) {
			size_t parentIndex = body.parentIndex;

			// trail relative to parent body
			if (parentIndex != -1) {
				if (trail->size() > 1) {
					bool doLoop = true;

					// remove any trail points behind the body's position
					while (doLoop) {
						glm::vec3 a = trail->front();
						glm::vec3 vel = body.velocity - bodies[parentIndex].velocity;
						glm::vec3 b = trail->back();

						glm::vec3 n = glm::cross(b, glm::vec3(body.position - bodies[parentIndex].position));

						// get angle between the last point added to the trail and the start of the trail
						double angle = acos(glm::dot(a, b) / (glm::length(a) * glm::length(b)));
						if (glm::dot(n, glm::cross(a, b)) < 0)
							angle *= -1;

						// get angle between the body's current position and the start of the trail
						b = body.position - bodies[parentIndex].position;
						double angle2 = acos(glm::dot(a, b) / (glm::length(a) * glm::length(b)));
						if (glm::dot(n, glm::cross(a, b)) < 0)
							angle2 *= -1;

						if (angle < 0 && angle2 >= 0)
							trail->pop_front();
						else
							doLoop = false;
					}
				}

				trail->push_back(glm::vec3(body.position - bodies[parentIndex].position));

			}

			// trail relative to world space
			else {
				trail->push_back(body.position);
				while (trail->size() > maxTrailLength) {
					trail->pop_front();
				}
			}
		}
	}
}

void cleanGL() {
	glDeleteBuffers(1, &trailVBO);
	glDeleteBuffers(1, &trailAlphaBuf);

	glDeleteProgram(shader.index);
	glDeleteProgram(skyboxShader.index);
	glDeleteProgram(trailShader.index);

	glfwTerminate();
}

void buildObjects() {
	GravityBodyBuilder builder;

	builder.buildSolarSystem(sphere);

	// !earth
	Orbit earthOrbit(&bodies[0], 1.494880e5, 0.01671123f, 1.796601f, 0.1745f, -2.672099e-7f, 0.0f);
	builder.init(5.9722e24f, earthOrbit, 0);
	builder.setModel(bodies[3].modelIndex);
	builder.setRadius(6.371f);
	double spin = 2 * pi / 86400;
	builder.setSpin(spin);
	builder.setOrientation(glm::dvec3(0.40910518 + earthOrbit.inclination, 0, 0));
	builder.setSurface(bodies[3].surface);
	builder.addTrail(glm::vec3(0.9f, 0.9f, 0.9f));
	bodies.push_back(builder.get());

	bodies[bodies.size() - 1].parentIndex = 0;
	builder.buildSky(cube);
}

void physicsLoop() {
	double lastLoopTime = glfwGetTime();
	while (running) {
		double currentTime = glfwGetTime();
		glm::float64 deltaTime = currentTime - lastLoopTime;
		lastLoopTime = currentTime;

		if (hasPhysics)
			updateBodies(deltaTime);

		flyCam(window, deltaTime);

		// manual control: adjust earth axial tilt and time of day
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			bodies[3].orientation.y += deltaTime;
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			bodies[3].orientation.y -= deltaTime;
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			bodies[3].orientation.x += deltaTime;
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			bodies[3].orientation.x -= deltaTime;

		// data is ready for renderer to access
		physicsUpdated = true;
		physicsFrames++;
		physicsCV.notify_one();
	}
}

void renderLoop(GLFWwindow* window) {
	double lastFrameTime = glfwGetTime();
	double currentTime = glfwGetTime();

	while (!glfwWindowShouldClose(window)) {
		if (currentTime - lastFrameTime > MIN_FRAME_TIME) {
			// capture physics results when they are ready
			{
				std::unique_lock<std::mutex> lock(physicsMutex);
				physicsCV.wait(lock, [] { return physicsUpdated; });
				physicsUpdated = false;
				lastPhysicsFrames = physicsFrames;
			}

			if (hasPhysics)
				updateTrails(currentTime - lastFrameTime);

			render();
			lastFrameTime = currentTime;

			glfwSwapBuffers(window);
			glfwPollEvents();
		}

		currentTime = glfwGetTime();
	}
}

int main() {
	initWindow();

#ifdef _DEBUG
	// enable OpenGL debug messages
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
#endif

	// initialize models
	cube = Model::Cube();
	sphere = Model::Icosphere(5);

	buildObjects();

	// setup starting camera
	camera.position = bodies[3].position + glm::dvec3(0.0, bodies[3].radius * 3, 0.0);
	camera.direction = glm::normalize(bodies[0].position - bodies[1].position);
	camera.right = glm::cross(camera.direction, glm::dvec3(0.0, 1.0, 0.0));
	camera.up = glm::cross(camera.right, camera.direction);

	setXY(window);

	// obtain initial perspective information from relationship between window and screen
	GLFWmonitor* screen = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(screen);
	screenSize = mode->height;
	projection = glm::perspective(FOV * HEIGHT / screenSize, (float)WIDTH / HEIGHT, 1e0f, 1e9f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	shader = initStandardShader();
	skyboxShader = initSkyboxShader();
	trailShader = initTrailShader();

	glGenVertexArrays(1, &trailVAO);
	glGenBuffers(1, &trailVBO);
	glGenBuffers(1, &trailAlphaBuf);

	initTime = glfwGetTime();

	// entering work area: split program into physics and rendering threads
	std::thread physicsThread(physicsLoop);
	renderLoop(window);

	// exiting work area: close threads and clean up data
	running = false;
	physicsThread.join();

	cleanGL();
	exit(EXIT_SUCCESS);
}