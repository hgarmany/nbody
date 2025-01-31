#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <chrono>
#include "physics.h"
#include "controls.h"

#include "stb_image.h"

Shader shader, skyboxShader;
glm::mat4 projection;
GLFWwindow* window;
Entity skybox;

size_t starBody;
size_t cube, sphere;
int screenSize = WIDTH;

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
	glm::vec3 lightPos = bodies[starBody].position;
	glm::vec3 lightColor = glm::vec3(1.0f);
	glUniform3fv(shader.uniforms[LIGHT_POS], 1, &lightPos[0]);
	glUniform3fv(shader.uniforms[LIGHT_COLOR], 1, &lightColor[0]);

	// set camera
	glm::mat4 view = camera.viewMatrix();
	setPV(shader, projection, view);

	for (Entity body : bodies)
		body.draw(shader, MODE_TEX);


	glDepthFunc(GL_LEQUAL);
	glUseProgram(skyboxShader.index);

	// set camera
	view = glm::mat4(glm::mat3(view));
	setPV(skyboxShader, projection, view);

	skybox.draw(skyboxShader, MODE_CUBEMAP);

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
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}

void buildObjects() {
	std::vector<std::string> faces = {
		"assets/sky/right.jpg",
		"assets/sky/left.jpg",
		"assets/sky/top.jpg",
		"assets/sky/bottom.jpg",
		"assets/sky/front.jpg",
		"assets/sky/back.jpg"
	};

	Surface earth = Surface("assets/earth.jpg", glm::vec4(0.0f, 1.0f, 1.0f, 0.0f), glm::vec3(1.0f));
	earth.normal = Surface::getTexture("assets/earth_normal.jpg");
	Surface moon = Surface("assets/moon.jpg", glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), glm::vec3(1.0f));
	moon.normal = Surface::getTexture("assets/moon_normal.jpg");
	Surface sun = Surface("assets/sun.jpg", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec3(1.0f));
	Surface stars = Surface::CubeMap(faces);

	// sun
	GravityBodyBuilder builder;
	builder.init(1.9891e30f);
	builder.setModel(sphere);
	builder.setRadius(500.0f);
	double spin = 2 * pi / 86400 / 27;
	builder.setMotion(glm::dvec3(0.0), glm::dvec3(0.0));
	builder.setOrientation(glm::dvec3(0.126, 0, 0));
	builder.setSurface(sun);
	bodies.push_back(builder.get());
	starBody = 0;

	// earth
	builder.init(5.9722e24f);
	builder.setModel(sphere);
	builder.setRadius(250.0f);
	spin = 2 * pi / 86400;
	builder.setMotion(
		glm::dvec3(149598.0, 0.0, 0.0), 
		glm::dvec3(0.0, 0.0, 0.029786), 
		glm::dvec3(0.0, spin, 0.0)
	);
	builder.setOrientation(glm::dvec3(0.40910518, 0, 0));
	builder.setSurface(earth);
	bodies.push_back(builder.get());

	//moon
	builder.init(7.3477e22f);
	builder.setModel(sphere);
	builder.setRadius(100.0f);
	spin = 2 * pi / 86400 / 27.3;
	builder.setMotion(
		glm::dvec3(149982.7, 0.0, 0.0), 
		glm::dvec3(0.0, 0.0, 0.028764), 
		glm::dvec3(0.0, spin, 0.0)
	);
	builder.setOrientation(glm::dvec3(0.116588, 0, 0));
	builder.setSurface(moon);
	bodies.push_back(builder.get());

	// !earth
	builder.init(5.9722e24f);
	builder.setModel(sphere);
	builder.setRadius(250.0f);
	spin = 2 * pi / 86400;
	builder.setMotion(
		glm::dvec3(147216.9, 0.0, 25958.3), 
		glm::dvec3(-0.0051742, 0.0, 0.029344), 
		glm::dvec3(0.0, spin, 0.0)
	);
	builder.setOrientation(glm::dvec3(0.40910518, 0, 0));
	builder.setSurface(earth);
	bodies.push_back(builder.get());

	// second sun
	/*builder.init(1.9891e30f);
	builder.setModel(sphere);
	builder.setRadius(500.0f);
	builder.setMotion(glm::vec3(0.0, 2500.0f, 0.0f), glm::vec3(0.25f, 0.0f, 0.0f));
	builder.setSurface(sun);
	bodies.push_back(builder.get());*/

	// stars
	EntityBuilder skyBuilder;
	skyBuilder.init();
	skyBuilder.setModel(cube);
	skyBuilder.setSurface(stars);
	skybox = skyBuilder.get();
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
	sphere = Model::Icosphere(4);

	buildObjects();

	// setup starting camera
	camera.position = bodies[1].position;
	camera.position.y += bodies[1].scale.y * 2;
	camera.direction = glm::normalize(bodies[0].position - bodies[1].position);
	camera.right = glm::cross(camera.direction, glm::dvec3(0.0, 1.0, 0.0));
	camera.up = glm::cross(camera.right, camera.direction);

	setXY(window);

	GLFWmonitor* screen = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(screen);
	screenSize = mode->height;
	projection = glm::perspective(FOV * HEIGHT / screenSize, (float)WIDTH / HEIGHT, 1e0f, 1e9f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	shader = initStandardShader();
	skyboxShader = initSkyboxShader();

	double lastLoopTime = glfwGetTime();
	double lastFrameTime = lastLoopTime;

	while (!glfwWindowShouldClose(window)) {
		double currentTime = glfwGetTime();
		glm::float64 deltaTime = currentTime - lastLoopTime;
		lastLoopTime = currentTime;

		if (hasPhysics)
			updateBodies(deltaTime);

		flyCam(window, deltaTime);

		// flip around sun-cube
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			bodies[1].orientation.y += deltaTime;
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			bodies[1].orientation.y -= deltaTime;
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			bodies[1].orientation.x += deltaTime;
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			bodies[1].orientation.x -= deltaTime;

		if (currentTime - lastFrameTime > MAX_FRAME_TIME) {
			//std::cout << currentTime - lastFrameTime << "\n";
			camera.position = bodies[1].position + glm::dvec3(0.0, 3.0, 0.0) * bodies[1].scale;

			render();
			lastFrameTime = currentTime;

			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}

	// Cleanup
	glDeleteProgram(shader.index);
	glDeleteProgram(skyboxShader.index);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
