#pragma once

#include <condition_variable>
#include <vector>
#include "gravitybody.h"

extern std::vector<GravityBody> bodies;

glm::dvec3 gravitationalForce(const GravityBody& a, const GravityBody& b);
void updateBodies(glm::float64 deltaTime, std::vector<GravityBody>& bodies);