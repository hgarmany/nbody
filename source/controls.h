#pragma once

#include "camera.h"
#include <map>
#include <GLFW/glfw3.h>

extern Camera camera, pipCam;
extern bool hasPhysics, overheadLock;
extern int windowWidth, windowHeight;
extern double lastX, lastY;
extern double timeStep; // Time step for the simulation;

enum keyMapName {
	MOVE_FORWARD, MOVE_BACKWARD, STRAFE_LEFT, STRAFE_RIGHT,
	PITCH_UP, PITCH_DOWN, YAW_LEFT, YAW_RIGHT, ROLL_LEFT, ROLL_RIGHT,
	T_PHYSICS, T_LOCK_SELECT, T_LOCK_PAGE_UP, T_LOCK_PAGE_DOWN, T_LOCK_OVERHEAD,
	INCREASE_TIME_STEP, DECREASE_TIME_STEP, SWAP_CAMERAS, SNAP_TO_TARGET,
	QUIT
};

void setXY(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void rollCamera(GLFWwindow* window, double deltaTime);
void flyCam(GLFWwindow* window, double deltaTime);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);