#pragma once

#include "shader.h"
#include "physics.h"

void initCamera();
void initQuad();
void initPIP();
void initShaders();
void initTrails();
void initStarBuffer();
void cleanGL();

void setPV(Shader& shader, glm::mat4& P, glm::mat4& V);
void updateProjectionMatrix(GLFWwindow* window);
void updateTrails(double time, camera_mode mode);
void drawQuad();
void render(Camera& camera);
void renderPIP(GLFWwindow* window);
void renderLoop(GLFWwindow* window);