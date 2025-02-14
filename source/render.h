#pragma once

#include "shader.h"
#include "physics.h"

extern glm::mat4 projection;
extern int screenSize;
extern float pipSize;

extern GLuint trailVAO, trailVBO, trailAlphaBuf, quadVAO, quadVBO, pipFBO, pipTexture, pipDepthBuffer;

void initShaders();
void cleanGL();

void setPV(Shader& shader, glm::mat4& P, glm::mat4& V);
void updateProjectionMatrix(GLFWwindow* window);
void updateTrails(double time, camera_mode mode);
void drawQuad();
void render(Camera& camera);
void renderPIP(GLFWwindow* window);
void renderLoop(GLFWwindow* window);