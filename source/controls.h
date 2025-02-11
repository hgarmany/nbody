#pragma once

#include "camera.h"
#include <map>
#include <GLFW/glfw3.h>

extern Camera camera;
extern bool hasPhysics;
extern int WIDTH, HEIGHT;

enum keyMapName {
	MOVE_FORWARD, MOVE_BACKWARD, STRAFE_LEFT, STRAFE_RIGHT,
	PITCH_UP, PITCH_DOWN, YAW_LEFT, YAW_RIGHT, ROLL_LEFT, ROLL_RIGHT
};

extern std::map<keyMapName, int> keyMap;
extern keyMapName mousePXAction, mouseNXAction, mousePYAction, mouseNYAction;

void setXY(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void flyCam(GLFWwindow* window, double deltaTime);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);