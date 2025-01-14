#pragma once

#include "gravitybody.h"

#include "physics.h"
#include <vector>

const double G = 6.67430e-29; // Gravitational constant
const double TIME_STEP = 2.5e5; // Time step for the simulation

extern std::vector<GravityBody> bodies;

glm::dvec3 gravitationalForce(const GravityBody& a, const GravityBody& b);
void updateBodies(glm::float64 deltaTime);