#pragma once

#include <GL/glew.h>
#include <map>

enum shaderUniform : uint8_t
{
    LIGHT_POS, LIGHT_COLOR, OBJ_COLOR, OBJ_MAT, OBJ_POS, OBJ_OBLATE, VIEW_POS, TEX_BOOL, NORM_BOOL, TEX_MAP, TEX_SIZE, NORMAL_MAP,
    SCREEN_WIDTH, WINDOW_SIZE, U_FOV
};

typedef struct shader {
    GLuint index, M, V, P;
    std::map<uint8_t, GLuint> uniforms;

    shader() : index(0), M(0), V(0), P(0) {};

    shader(const shader&) = delete;
    shader& operator=(const shader&) = delete;

    shader(shader&& other) noexcept {
        index = other.index;
        M = other.M;
        V = other.V;
        P = other.P;
        uniforms = std::move(other.uniforms);

        other.index = 0;
    }

    shader& operator=(shader&& other) noexcept {
        if (this != &other) {
            index = other.index;
            M = other.M;
            V = other.V;
            P = other.P;
            uniforms = std::move(other.uniforms);

            other.index = 0;
        }
        return *this;
    }

    ~shader() {
        if (index)
            glDeleteProgram(index);
    }
} Shader;

Shader initStandardShader();
Shader initSkyboxShader();
Shader initTrailShader();
Shader initFrameOverlayShader();
Shader initSpriteShader();