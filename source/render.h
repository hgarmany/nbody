#pragma once

#include "shader.h"
#include "physics.h"

void initWindow();
void initCamera();
void initQuad();
void initShadowMap();
void initPIP();
void initImGui();
void initShaders();
void initTrails();
void initStarBuffer();
void window_size_callback(GLFWwindow* window, int width, int height);
void cleanup();

void renderLoop();