#pragma once

#include <GL/glew.h>
#include <map>

enum shaderUniform : uint8_t
{
    LIGHT_POS, LIGHT_COLOR, OBJ_COLOR, OBJ_MAT, OBJ_POS, VIEW_POS, TEX_BOOL, NORM_BOOL, TEX_MAP, NORMAL_MAP
};

typedef struct shader {
    GLuint index = 0, M = 0, V = 0, P = 0;
    std::map<uint8_t, GLuint> uniforms;

} Shader;

Shader initStandardShader();
Shader initSkyboxShader();
Shader initTrailShader();
Shader initFrameOverlayShader();