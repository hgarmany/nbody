#pragma once

#include <condition_variable>
#include <thread>
#include <vector>
#include "gravitybody.h"
#include "controls.h"

extern std::atomic<bool> running;
extern std::condition_variable physicsCV;
extern bool physicsUpdated;
extern int physicsFrames, lastPhysicsFrames;

extern std::vector<GravityBody> bodies, frameBodies;

glm::dvec3 gravitationalForce(const GravityBody& a, const GravityBody& b);
void updateBodies(glm::float64 deltaTime, std::vector<GravityBody>& bodies);
void physicsLoop(GLFWwindow* window);