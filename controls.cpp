#include "controls.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

Camera camera;
bool hasPhysics = false;
bool firstMouse = true;
bool cursorDisabled = true;

glm::float64 pitch, yaw, roll;

double lastX = WIDTH / 2;
double lastY = HEIGHT / 2;

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
};

keyMapName mousePXAction = YAW_LEFT, mouseNXAction = YAW_RIGHT, mousePYAction = PITCH_UP, mouseNYAction = PITCH_DOWN;

// Camera movement speed
double cameraSpeed = 100.0f;

void setXY(GLFWwindow* window) {
	glfwGetCursorPos(window, &lastX, &lastY);
}

// Function to update the camera direction based on mouse input
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



// Function to move the camera based on keyboard input
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_P:
			hasPhysics = !hasPhysics;
		}
	}

	if (action == GLFW_RELEASE) {
		/*switch (key) {
		default:
			break;
		}*/
	}
}

// Function to move the camera based on keyboard input
void flyCam(GLFWwindow* window, double deltaTime) {
	glm::float64 velocity = cameraSpeed * deltaTime;

	if (glfwGetKey(window, keyMap[MOVE_FORWARD]) == GLFW_PRESS)
		camera.position += camera.direction * velocity;
	if (glfwGetKey(window, keyMap[STRAFE_LEFT]) == GLFW_PRESS)
		camera.position -= camera.right * velocity;
	if (glfwGetKey(window, keyMap[MOVE_BACKWARD]) == GLFW_PRESS)
		camera.position -= camera.direction * velocity;
	if (glfwGetKey(window, keyMap[STRAFE_RIGHT]) == GLFW_PRESS)
		camera.position += camera.right * velocity;
	if (glfwGetKey(window, keyMap[PITCH_UP]) == GLFW_PRESS)
		camera.setOrientation(2.0f * deltaTime, 0.0f, 0.0f);
	if (glfwGetKey(window, keyMap[PITCH_DOWN]) == GLFW_PRESS)
		camera.setOrientation(-2.0f * deltaTime, 0.0f, 0.0f);
	if (glfwGetKey(window, keyMap[YAW_LEFT]) == GLFW_PRESS)
		camera.setOrientation(0.0f, 2.0f * deltaTime, 0.0f);
	if (glfwGetKey(window, keyMap[YAW_RIGHT]) == GLFW_PRESS)
		camera.setOrientation(0.0f, -2.0f * deltaTime, 0.0f);

	if (glfwGetKey(window, keyMap[ROLL_LEFT]) == GLFW_PRESS)
		camera.setOrientation(0.0f, 0.0f, -2.0f * deltaTime);
	if (glfwGetKey(window, keyMap[ROLL_RIGHT]) == GLFW_PRESS)
		camera.setOrientation(0.0f, 0.0f, 2.0f * deltaTime);
}

// Function to toggle cursor visibility
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		cursorDisabled = !cursorDisabled;

		if (cursorDisabled)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide and capture cursor
		else
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // Enable cursor
	}
}

// Scroll callback function
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	if (yoffset > 0) {
		cameraSpeed *= 0.8f; // Decrease speed when scrolling up
	}
	else {
		cameraSpeed *= 1.2f; // Increase speed when scrolling down
	}
}