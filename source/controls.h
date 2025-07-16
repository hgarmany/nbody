#pragma once

#include "camera.h"
#include <map>
#include <GLFW/glfw3.h>

extern bool overheadLock, showWelcomeMenu, showLockIndexMenu, showSettingsMenu, doStarSprites;
extern int windowWidth, windowHeight;
extern double lastX, lastY, deltaTime;

void setXY(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void rollCamera(Camera& camera, GLFWwindow* window);
void flyCam(GLFWwindow* window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);