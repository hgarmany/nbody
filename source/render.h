#pragma once

#include "shader.h"
#include "physics.h"

extern GLFWwindow* window;

void initCamera();
void initQuad();
void initPIP();
void initShaders();
void initTrails();
void initStarBuffer();
void cleanGL();

void updateProjectionMatrix();
void renderLoop();