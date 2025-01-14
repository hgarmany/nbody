#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <chrono>
#include "physics.h"
#include "controls.h"

#include "stb_image.h"

Shader shader, skyboxShader;
glm::mat4 projection = glm::perspective(glm::radians(30.0f), (float)WIDTH / HEIGHT, 1e2f, 1e9f);
GLFWwindow* window;
Entity skybox;
Surface earth, sun, moon;

// Camera and lighting
glm::vec3 lightPos;
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
glm::vec3 objectColor(1.0f, 0.5f, 0.31f);

// Function to update the projection matrix based on window size
void static updateProjectionMatrix(GLFWwindow* window) {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);  // Get the current framebuffer size

	if (height == 0) height = 1;  // Prevent division by zero

	// Calculate the aspect ratio dynamically
	float aspect = (float)width / (float)height;

	// Define the projection matrix (FOV, aspect ratio, near, far)
	projection = glm::perspective(glm::radians(30.0f), aspect, 1e2f, 1e9f);
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
	glfwWindowHint(GLFW_DEPTH_BITS, 32);

	glewInit();

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback); // Set scroll callback
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetKeyCallback(window, key_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide and capture cursor initially
}

// Load a cubemap texture from 6 individual texture faces
unsigned int static loadCubemap() {
	// Load the cubemap textures
	std::vector<std::string> faces = {
		"assets/right.jpg",
		"assets/left.jpg",
		"assets/top.jpg",
		"assets/bottom.jpg",
		"assets/front.jpg",
		"assets/back.jpg"
	};

	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++) {
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else {
			std::cerr << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void static renderSkybox(unsigned int skyboxVAO, unsigned int cubemapTexture) {
	glDepthFunc(GL_LEQUAL);
	glUseProgram(skyboxShader.index);

	glm::mat4 view = glm::mat4(glm::mat3(camera.viewMatrix()));
	glUniformMatrix4fv(skyboxShader.P, 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(skyboxShader.V, 1, GL_FALSE, &view[0][0]);

	glBindVertexArray(skyboxVAO);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glDepthFunc(GL_LESS);
}

void static render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Activate the shader program
	glUseProgram(shader.index);
	glUniform3fv(shader.lightPos, 1, &lightPos[0]);
	glUniform3fv(shader.lightColor, 1, &lightColor[0]);
	glUniformMatrix4fv(shader.P, 1, GL_FALSE, &projection[0][0]);

	// Set the camera and light properties
	glm::mat4 view = camera.viewMatrix();
	glUniformMatrix4fv(shader.V, 1, GL_FALSE, &view[0][0]);

	for (Entity body : bodies)
		body.draw(shader);
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

void buildObjects(std::shared_ptr<Model> sphere, std::shared_ptr<Model> cube) {
	earth = Surface("assets/earth.jpg", glm::vec4(0.0f, 1.0f, 1.0f, 0.0f), glm::vec3(1.0f));
	moon = Surface("assets/moon.jpg", glm::vec4(0.0f, 1.0f, 1.0f, 0.0f), glm::vec3(1.0f));
	sun = Surface("assets/sun.jpg", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec3(1.0f));

	// sun
	GravityBodyBuilder builder;
	builder.init(1.9891e30f);
	builder.setModel(sphere);
	builder.setRadius(500.0f);
	builder.setMotion(glm::vec3(0.0), glm::vec3(0.0));
	builder.setSurface(sun);
	bodies.push_back(builder.get());

	// earth
	builder.init(5.9722e24f);
	builder.setModel(sphere);
	builder.setRadius(250.0f);
	builder.setMotion(glm::vec3(149598.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.029786));
	builder.setSurface(earth);
	bodies.push_back(builder.get());

	//moon
	builder.init(7.3477e22f);
	builder.setModel(sphere);
	builder.setRadius(100.0f);
	builder.setMotion(glm::vec3(149982.7, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.030808));
	builder.setSurface(moon);
	bodies.push_back(builder.get());

	// !earth
	builder.init(5.9722e24f);
	builder.setModel(sphere);
	builder.setRadius(250.0f);
	//builder.setMotion(glm::vec3(0.0, 0.0, 149488.0), glm::vec3(-0.029797, 0.0, 0.0));
	builder.setMotion(glm::vec3(147216.9, 0.0, 25958.3), glm::vec3(-0.0051742, 0.0, 0.029344));
	builder.setSurface(earth);
	bodies.push_back(builder.get());
}

int main() {
	initWindow();

	// During init, enable debug output
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	shader = initStandardShader();
	skyboxShader = initSkyboxShader();

	// Generate the sphere's vertices and indices
	Model c_model = Model::Cube();
	Model s_model = Model::Icosphere(3);
	std::shared_ptr<Model> cube = std::make_shared<Model>(c_model);
	std::shared_ptr<Model> sphere = std::make_shared<Model>(s_model);

	unsigned int cubemapTexture = loadCubemap();

	skybox.model = cube;
	buildObjects(sphere, cube);

	// setup starting camera
	camera.position = bodies[1].position;
	camera.position.y += bodies[1].scale.y * 2;
	camera.direction = glm::normalize(bodies[0].position - bodies[1].position);
	camera.right = glm::cross(camera.direction, glm::dvec3(0.0, 1.0, 0.0));
	camera.up = glm::cross(camera.right, camera.direction);

	setXY(window);

	glEnable(GL_DEPTH_TEST);

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
			bodies[0].orientation.z += deltaTime;
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			bodies[0].orientation.z -= deltaTime;
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			bodies[0].orientation.x += deltaTime;
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			bodies[0].orientation.x -= deltaTime;

		lightPos = bodies[0].position;
		lightPos.y += 10000;

		if (currentTime - lastFrameTime > MAX_FRAME_TIME) {
			//std::cout << currentTime - lastFrameTime << "\n";
			render();
			renderSkybox(skybox.model->VAO, cubemapTexture);
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
