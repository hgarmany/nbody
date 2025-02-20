#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <chrono>
#include "builder.h"
#include "render.h"

#include "stb_image.h"

// Set this function as a callback to update projection matrix during window resizing
void static window_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);  // Set the OpenGL viewport to match the new window size
	windowWidth = width;
	windowHeight = height;
	updateProjectionMatrix();  // Update the projection matrix with the new size
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
	size_t cube, sphere;

	// initialize models
	cube = Model::Cube();
	sphere = Model::Icosphere(5);

	GravityBodyBuilder builder;

	builder.buildSolarSystem(sphere);
	
	// !earth
	Orbit earthOrbit(&bodies[0], 1.494880e5, 0.01671123f, 1.796601f, 0.1745f, -2.672099e-7f, 0.0f);
	builder.init(5.9722e24f, earthOrbit, 0);
	builder.setModel(sphere);
	builder.setRadius(6.371f);
	double spin = 2 * pi / 86400;
	builder.setSpin(spin);
	builder.setOrientation(glm::dvec3(0.40910518 + earthOrbit.inclination, 0, 0));
	builder.setSurface(bodies[3].surface);
	builder.addTrail(glm::vec3(0.9f, 0.9f, 0.9f), 3); // orbital trail w.r.t earth
	bodies.push_back(builder.get());

	builder.buildSky(cube);

	// camera
	builder.init();
	builder.setMotion(bodies[0].position + glm::dvec3(0, 0, bodies[0].radius * 5), bodies[0].velocity * 1.1);
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
	
	buildObjects();

	initCamera();

	setXY(window);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	initShaders();
	initTrails();
	initStarBuffer();

	// entering work area: split program into physics and rendering threads
	std::thread physicsThread(physicsLoop, window);
	renderLoop();

	// exiting work area: close threads and clean up data
	running = false;
	physicsThread.join();

	cleanGL();
	exit(EXIT_SUCCESS);
}