#pragma once

#include <functional>
#include <glm.hpp>

const double pi = 3.141592653589793;
const float pi_f = 3.141592653589793f;
const float MIN_FRAME_TIME = 1.0f / 60.0f;

// units : space in Mm, time in s
const double G = 6.67430e-29; // Gravitational constant

enum render_mode : uint8_t {
	MODE_TEX,
	MODE_SOLID,
	MODE_CUBEMAP
};

enum camera_mode : uint8_t {
	FREE_CAM,
	LOCK_PLANET_CAM,
	LOCK_BARY_CAM,
	GRAV_CAM
};

enum astronomical_data : uint8_t {
	SEMI_MAJOR_AXIS,
	ECCENTRICITY,
	ARGUMENT_PERIAPSIS,
	ASCENDING_NODE_LONGITUDE,
	INCLINATION,
	MEAN_ANOMALY,
	MASS,
	RADIUS
};

inline void printMatrix(const glm::dmat4& A) {
	printf("%.2e\t%.2e\t%.2e\t%.2e\n"
		"%.2e\t%.2e\t%.2e\t%.2e\n"
		"%.2e\t%.2e\t%.2e\t%.2e\n"
		"%.2e\t%.2e\t%.2e\t%.2e\n",
		A[0][0], A[0][1], A[0][2], A[0][3],
		A[1][0], A[1][1], A[1][2], A[1][3],
		A[2][0], A[2][1], A[2][2], A[2][3],
		A[3][0], A[3][1], A[3][2], A[3][3]);
}

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

size_t getIntsFromString(const char* string, int* out, size_t n, char dem = ' ');
size_t getFloatsFromString(const char* string, float* out, size_t n, char dem = ' ');
double ellipsePerimeter(double semiMajorAxis, float eccentricity);