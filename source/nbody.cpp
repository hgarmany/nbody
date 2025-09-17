#include "builder.h"
#include "render.h"
#include "controls.h"

static void MessageCallback(GLenum source,
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

// an ordered series of initialization requests
static void initSeries() {
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

	initShaders();
	//initShadowMap();
	initTrails();
	initStarBuffer();
	initImGui();

	//initLoggers();
}

int main() {
	initSeries();

	// entering work area: split program into physics and rendering threads
	std::thread physicsThread(physicsLoop);
	renderLoop();

	// exiting work area: close threads and clean up data
	running = false;
	physicsStart.notify_one();
	physicsThread.join();

	cleanup();
	exit(EXIT_SUCCESS);
}

int WinMain() {
	main();
}