#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <chrono>
#include "builder.h"
#include "render.h"

#include "stb_image.h"

GLFWwindow* window;

size_t cube, sphere;

double initTime;

// Set this function as a callback to update projection matrix during window resizing
void static window_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);  // Set the OpenGL viewport to match the new window size
	windowWidth = width;
	windowHeight = height;
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

	window = glfwCreateWindow(windowWidth, windowHeight, "N-Body Simulator", nullptr, nullptr);

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

// setup for the simple display quad
void initQuad() {
	GLfloat quadVertices[] = {
		// Positions       // Texture Coords
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // Bottom-left
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  // Bottom-right
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // Top-right
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // Top-right
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // Top-left
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f   // Bottom-left
	};

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);

	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	// load vertex data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// load uv mapping
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
}

// prepares an FBO for capturing renders
void initPIP() {
	glGenFramebuffers(1, &pipFBO);
	glGenTextures(1, &pipTexture);
	glGenRenderbuffers(1, &pipDepthBuffer);

	GLsizei pipWidth = GLsizei(windowWidth * pipSize);
	GLsizei pipHeight = GLsizei(windowHeight * pipSize);

	// texture space for holding render data
	glBindTexture(GL_TEXTURE_2D, pipTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pipWidth, pipHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// attach texture buffer
	glBindFramebuffer(GL_FRAMEBUFFER, pipFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pipTexture, 0);

	// attach depth buffer
	glBindRenderbuffer(GL_RENDERBUFFER, pipDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, pipWidth, pipHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pipDepthBuffer);
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

	// camera
	builder.init();
	builder.setMotion(bodies[3].position + bodies[3].radius * 5, bodies[3].velocity * 1.1);
	builder.addTrail();
	bodies.push_back(builder.get());
}

int main() {
	initWindow();

#ifdef _DEBUG
	// enable OpenGL debug messages
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
#endif
	initPIP();
	initQuad();

	// initialize models
	cube = Model::Cube();
	sphere = Model::Icosphere(5);

	buildObjects();

	// setup starting camera
	camera = Camera(
		bodies[3].position + glm::dvec3(0.0, bodies[3].radius * 3, 0.0),
		glm::normalize(bodies[0].position - bodies[1].position),
		glm::dvec3(0.0, 1.0, 0.0)
	);
	camera.mode = FREE_CAM;
	camera.mode = LOCK_CAM;

	setXY(window);

	// obtain initial perspective information from relationship between window and screen
	GLFWmonitor* screen = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(screen);
	screenSize = mode->height;
	projection = glm::perspective(FOV * windowHeight / screenSize, (float)windowWidth / windowHeight, 1e0f, 1e9f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	initShaders();

	glGenVertexArrays(1, &trailVAO);
	glGenBuffers(1, &trailVBO);
	glGenBuffers(1, &trailAlphaBuf);

	initTime = glfwGetTime();

	// entering work area: split program into physics and rendering threads
	std::thread physicsThread(physicsLoop, window);
	renderLoop(window);

	// exiting work area: close threads and clean up data
	running = false;
	physicsThread.join();

	cleanGL();
	exit(EXIT_SUCCESS);
}