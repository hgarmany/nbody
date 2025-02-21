#include "controls.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

Camera camera;
Camera pipCam(
	glm::dvec3(0, 1e6, 0),
	glm::dvec3(0, -1, 0),
	glm::dvec3(1, 0, 0));

bool firstMouse = true;
bool cursorDisabled = true;
bool hasPhysics = false;
bool cameraInertia = false;
bool overheadLock = false;

glm::float64 pitch, yaw, roll;

int windowWidth = 900, windowHeight = 600;

double lastX = windowWidth / 2;
double lastY = windowHeight / 2;

bool isChoosingBody = true;
double timeStep = 1e5;

std::map<keyMapName, int> keyMap = {
	{ MOVE_FORWARD, GLFW_KEY_W },
	{ MOVE_BACKWARD, GLFW_KEY_S },
	{ STRAFE_LEFT, GLFW_KEY_A },
	{ STRAFE_RIGHT, GLFW_KEY_D },
	{ PITCH_UP, -1 },
	{ PITCH_DOWN, -1 },
	{ YAW_LEFT, -1 },
	{ YAW_RIGHT, -1 },
	{ ROLL_LEFT, GLFW_KEY_Q },
	{ ROLL_RIGHT, GLFW_KEY_E },
	{ T_PHYSICS, GLFW_KEY_P },
	{ T_LOCK_SELECT, GLFW_KEY_F },
	{ T_LOCK_PAGE_UP, GLFW_KEY_RIGHT_BRACKET },
	{ T_LOCK_PAGE_DOWN, GLFW_KEY_LEFT_BRACKET },
	{ T_LOCK_OVERHEAD, GLFW_KEY_L },
	{ INCREASE_TIME_STEP, GLFW_KEY_PERIOD },
	{ DECREASE_TIME_STEP, GLFW_KEY_COMMA },
	{ SWAP_CAMERAS, GLFW_KEY_SPACE },
	{ SNAP_TO_TARGET, GLFW_KEY_G },
	{ QUIT, GLFW_KEY_ESCAPE }
};

// lambda library for all key-based controls
std::unordered_map<int, std::function<void()>> keyActions = {
		{keyMap[T_PHYSICS], []() { hasPhysics = !hasPhysics; }},
		{keyMap[T_LOCK_SELECT], []() {
			isChoosingBody = !isChoosingBody;
			camera.mode = (camera.mode == LOCK_CAM) ? FREE_CAM : LOCK_CAM;
			if (camera.mode == LOCK_CAM) camera.lockIndex = -1;
		}},
		{keyMap[T_LOCK_PAGE_UP], []() { camera.lockIndex++; }},
		{keyMap[T_LOCK_PAGE_DOWN], []() { camera.lockIndex--; }},
		{keyMap[T_LOCK_OVERHEAD], []() { overheadLock = !overheadLock; }},
		{keyMap[INCREASE_TIME_STEP], []() { timeStep *= 1.1; }},
		{keyMap[DECREASE_TIME_STEP], []() { timeStep *= 0.9; }},
		{keyMap[SWAP_CAMERAS], []() {
			std::swap(camera, pipCam);
			isChoosingBody = camera.mode == LOCK_CAM;
		}},
		{keyMap[SNAP_TO_TARGET], []() { camera.lockDistanceFactor = 5.0f; }},
		{keyMap[QUIT], []() { glfwSetWindowShouldClose(glfwGetCurrentContext(), true); }}
};

keyMapName mousePXAction = YAW_LEFT, mouseNXAction = YAW_RIGHT, mousePYAction = PITCH_UP, mouseNYAction = PITCH_DOWN;

// Camera movement speed
double cameraSpeed = 1e5;

void setXY(GLFWwindow* window) {
	glfwGetCursorPos(window, &lastX, &lastY);
}

// update the camera orientation based on mouse input
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (cursorDisabled) {
		float xOffset = (float)(lastX - xpos);
		float yOffset = (float)(lastY - ypos);

		pitch = yaw = roll = 0.0;

		switch (mousePXAction) {
		case YAW_LEFT:
			yaw += xOffset;
			break;
		case YAW_RIGHT:
			yaw -= xOffset;
			break;
		case ROLL_LEFT:
			roll -= xOffset;
			break;
		case ROLL_RIGHT:
			roll += xOffset;
			break;
		}

		switch (mousePYAction) {
		case PITCH_UP:
			pitch += yOffset;
			break;
		case PITCH_DOWN:
			pitch -= yOffset;
			break;
		}

		camera.setOrientation(pitch, yaw, roll);
	}

	lastX = xpos;
	lastY = ypos;
}



// process key presses and releases
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action != GLFW_PRESS) {
		if (keyActions.find(key) != keyActions.end())
			keyActions[key]();

		// handling for body index selection
		if (isChoosingBody && key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
			camera.lockIndex = key - GLFW_KEY_0;
	}
}

void rollCamera(GLFWwindow* window, double deltaTime) {
	if (glfwGetKey(window, keyMap[ROLL_LEFT]) == GLFW_PRESS)
		camera.setOrientation(0.0f, 0.0f, -deltaTime * 1e3);
	if (glfwGetKey(window, keyMap[ROLL_RIGHT]) == GLFW_PRESS)
		camera.setOrientation(0.0f, 0.0f, deltaTime * 1e3);
}

// Function to move the camera based on keyboard input
void flyCam(GLFWwindow* window, double deltaTime) {
	glm::float64 speed = cameraSpeed * deltaTime;

	if (cameraInertia == false)
		camera.velocity = glm::dvec3(0.0);

	rollCamera(window, deltaTime);

	if (glfwGetKey(window, keyMap[PITCH_UP]) == GLFW_PRESS)
		camera.setOrientation(deltaTime, 0.0f, 0.0f);
	if (glfwGetKey(window, keyMap[PITCH_DOWN]) == GLFW_PRESS)
		camera.setOrientation(-deltaTime, 0.0f, 0.0f);
	if (glfwGetKey(window, keyMap[YAW_LEFT]) == GLFW_PRESS)
		camera.setOrientation(0.0f, deltaTime, 0.0f);
	if (glfwGetKey(window, keyMap[YAW_RIGHT]) == GLFW_PRESS)
		camera.setOrientation(0.0f, -deltaTime, 0.0f);
	if (glfwGetKey(window, keyMap[MOVE_FORWARD]) == GLFW_PRESS)
		camera.velocity += camera.direction * speed;
	if (glfwGetKey(window, keyMap[STRAFE_LEFT]) == GLFW_PRESS)
		camera.velocity -= camera.right * speed;
	if (glfwGetKey(window, keyMap[MOVE_BACKWARD]) == GLFW_PRESS)
		camera.velocity -= camera.direction * speed;
	if (glfwGetKey(window, keyMap[STRAFE_RIGHT]) == GLFW_PRESS)
		camera.velocity += camera.right * speed;

	camera.position += camera.velocity * deltaTime;
}

// toggle between usable cursor and mouse-controlled camera
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		cursorDisabled = !cursorDisabled;

		if (cursorDisabled)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // hide and capture cursor
		else
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // enable cursor
	}
}

// mouse scroll processing
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	if (yoffset > 0) {
		if (camera.mode == LOCK_CAM)
			camera.lockDistanceFactor *= 0.9f; // zoom in
		else
			cameraSpeed *= 0.8f; // decrease speed when scrolling up
	}
	else {
		if (camera.mode == LOCK_CAM)
			camera.lockDistanceFactor *= 1.1f; // zoom out
		else
			cameraSpeed *= 1.2f; // increase speed when scrolling down
	}
}