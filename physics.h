#pragma once

#include "gravitybody.h"

#include "physics.h"
#include <vector>

extern std::vector<GravityBody> bodies;

glm::dvec3 gravitationalForce(const GravityBody& a, const GravityBody& b);
void updateBodies(glm::float64 deltaTime, std::vector<GravityBody>& bodies);