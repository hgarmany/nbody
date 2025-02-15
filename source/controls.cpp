#include "controls.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

Camera camera;
bool firstMouse = true;
bool cursorDisabled = true;
bool hasPhysics = false;

glm::float64 pitch, yaw, roll;

int windowWidth = 900, windowHeight = 600;

double lastX = windowWidth / 2;
double lastY = windowHeight / 2;

bool isChoosingBody = true;
size_t lockIndex = 0;
double lockDistanceFactor = 5;
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
	{ INCREASE_TIME_STEP, GLFW_KEY_PERIOD },
	{ DECREASE_TIME_STEP, GLFW_KEY_COMMA }
};

keyMapName mousePXAction = YAW_LEFT, mouseNXAction = YAW_RIGHT, mousePYAction = PITCH_UP, mouseNYAction = PITCH_DOWN;

// Camera movement speed
double cameraSpeed = 2e2;

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
	if (action == GLFW_PRESS) {
		if (key == keyMap[T_PHYSICS])
			hasPhysics = !hasPhysics;
		else if (key == keyMap[T_LOCK_SELECT]) {
			isChoosingBody = !isChoosingBody;
			if (camera.mode == LOCK_CAM)
				camera.mode = FREE_CAM;
			else {
				camera.mode = LOCK_CAM;
				lockIndex = -1;
			}
		}
		else if (key == keyMap[T_LOCK_PAGE_UP])
			lockIndex++;
		else if (key == keyMap[T_LOCK_PAGE_DOWN])
			lockIndex--;
		else if (key == keyMap[INCREASE_TIME_STEP])
			timeStep *= 1.1;
		else if (key == keyMap[DECREASE_TIME_STEP])
			timeStep *= 0.9;

		if (isChoosingBody && key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
			lockIndex = key - GLFW_KEY_0;
		}

	}
}

// Function to move the camera based on keyboard input
void flyCam(GLFWwindow* window, double deltaTime) {
	glm::float64 velocity = cameraSpeed * deltaTime;

	if (glfwGetKey(window, keyMap[ROLL_LEFT]) == GLFW_PRESS)
		camera.setOrientation(0.0f, 0.0f, -deltaTime * 1e3);
	if (glfwGetKey(window, keyMap[ROLL_RIGHT]) == GLFW_PRESS)
		camera.setOrientation(0.0f, 0.0f, deltaTime * 1e3);

	if (glfwGetKey(window, keyMap[PITCH_UP]) == GLFW_PRESS)
		camera.setOrientation(deltaTime, 0.0f, 0.0f);
	if (glfwGetKey(window, keyMap[PITCH_DOWN]) == GLFW_PRESS)
		camera.setOrientation(-deltaTime, 0.0f, 0.0f);
	if (glfwGetKey(window, keyMap[YAW_LEFT]) == GLFW_PRESS)
		camera.setOrientation(0.0f, deltaTime, 0.0f);
	if (glfwGetKey(window, keyMap[YAW_RIGHT]) == GLFW_PRESS)
		camera.setOrientation(0.0f, -deltaTime, 0.0f);
	if (glfwGetKey(window, keyMap[MOVE_FORWARD]) == GLFW_PRESS)
		camera.velocity += camera.direction * velocity;
		//camera.position += camera.direction * velocity;
	if (glfwGetKey(window, keyMap[STRAFE_LEFT]) == GLFW_PRESS)
		camera.velocity -= camera.right * velocity;
		//camera.position -= camera.right * velocity;
	if (glfwGetKey(window, keyMap[MOVE_BACKWARD]) == GLFW_PRESS)
		camera.velocity -= camera.direction * velocity;
		//camera.position -= camera.direction * velocity;
	if (glfwGetKey(window, keyMap[STRAFE_RIGHT]) == GLFW_PRESS)
		camera.velocity += camera.right * velocity;
		//camera.position += camera.right * velocity;
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
			lockDistanceFactor *= 0.9; // zoom in
		else
			cameraSpeed *= 0.8f; // decrease speed when scrolling up
	}
	else {
		if (camera.mode == LOCK_CAM)
			lockDistanceFactor *= 1.1; // zoom out
		else
			cameraSpeed *= 1.2f; // increase speed when scrolling down
	}
}