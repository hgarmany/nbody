#include "builder.h"
#include "render.h"
#include "controls.h"

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
	// initialize models
	size_t cube = Model::Cube();
	size_t sphere = Model::Icosphere(5);

	GravityBodyBuilder builder;

	//builder.buildSolarSystem(sphere);
	builder.buildAlienSystem(sphere);

	/*
	// cross section of ring structure
	std::vector<GLfloat> ringCut = {
		-0.2f, 1.5f, 0.0f,
		-0.35f, 1.65f, 0.0f,
		-0.9f, 1.7f, 0.0f,
		-1.0f, 1.75f, 0.0f,
		-0.05f, 1.8f, 0.0f,
		0.05f, 1.8f, 0.0f,
		0.2f, 2.0f, 0.0f,
		0.95f, 3.0f, 0.0f,
		1.0f, 2.95f, 0.0f,
		0.35f, 1.9f, 0.0f,
		0.05f, 1.5f, 0.0f,
		-0.05f, 1.45f, 0.0f,
		-0.05f, -1.45f, 0.0f,
		0.05f, -1.5f, 0.0f,
		0.35f, -1.9f, 0.0f,
		1.0f, -2.95f, 0.0f,
		0.95f, -3.0f, 0.0f,
		0.2f, -2.0f, 0.0f,
		0.05f, -1.8f, 0.0f,
		-0.05f, -1.8f, 0.0f,
		-1.0f, -1.75f, 0.0f,
		-0.9f, -1.7f, 0.0f,
		-0.35f, -1.65f, 0.0f,
		-0.2f, -1.5f, 0.0f
	};

	size_t ring = Model::Ring(ringCut, 900, 5e-2f);

	// ring
	Orbit ringOrbit(bodies[3], 1e1, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f);
	builder.init(1e21f, ringOrbit, 3);
	builder.setModel(ring);
	builder.setRadius(1.0f);
	builder.setSurface(Surface(glm::vec4(0.1f, 1.0f, 0.2f, 0.0f), glm::vec3(0.9f, 0.2f, 0.05f)));
	builder.setRotation(glm::dvec3(0.40910518 + ringOrbit.inclination, 0, 0));
	builder.addTrail(glm::vec3(0.0f, 1.0f, 0.0f)); // orbital trail w.r.t earth
	bodies.push_back(builder.get());
	*/

	// camera
	builder.init();
	builder.setMotion(bodies[0]->position + glm::dvec3(0, 0, bodies[0]->radius * 5), bodies[0]->velocity * 1.1);
	builder.addTrail(glm::vec3(1.0f, 1.0f, 0.0f));
	bodies.push_back(builder.get());

	builder.buildSky(cube);
	
	frameBodies = bodies;
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

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

	initShaders();
	initShadowMap();
	initTrails();
	initStarBuffer();
	initImGui();

	// entering work area: split program into physics and rendering threads
	std::thread physicsThread(physicsLoop);
	renderLoop();

	// exiting work area: close threads and clean up data
	running = false;
	physicsThread.join();

	cleanup();
	exit(EXIT_SUCCESS);
}

int WinMain() {
	main();
}