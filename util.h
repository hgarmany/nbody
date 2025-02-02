#pragma once

#include <functional>

const float pi = 3.141592653589793f;
const int WIDTH = 900, HEIGHT = 600;
const float MAX_FRAME_TIME = 1.0f / 60.0f;
const float FOV = glm::radians(45.0f);

// units : space in Mm, time in s
const double G = 6.67430e-29; // Gravitational constant
const double TIME_STEP = 1e6; // Time step for the simulation

enum render_mode : uint8_t {
	MODE_TEX,
	MODE_SOLID,
	MODE_CUBEMAP
};

template <typename T> T rootSolver(
    std::function<T(T)> func, std::function<T(T)> derivFunc, T x
) {
    T h = func(x) / derivFunc(x);
    while (abs(h) >= 0.0001) {
        h = func(x) / derivFunc(x); 
        x -= h;
    }

    return x;
}