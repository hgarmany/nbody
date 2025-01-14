#pragma once

#include <GL/glew.h>

typedef struct shaderInfo {
    GLuint index;
    GLuint M;
    GLuint V;
    GLuint P;
    GLuint lightPos;
    GLuint lightColor;
    GLuint objectColor;
    GLuint objectMat;
    GLuint viewPos;
    GLuint texBool;
    GLuint misc;
} Shader;

Shader initStandardShader();
Shader initSkyboxShader();