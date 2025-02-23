#pragma once

#include <condition_variable>
#include <thread>
#include <vector>
#include "camera.h"
#include "gravitybody.h"

extern Camera camera, pipCam;
extern std::atomic<bool> running;
extern std::condition_variable physicsCV;
extern bool hasPhysics, physicsUpdated;
extern double elapsedTime, timeStep;

extern std::vector<GravityBody> bodies, frameBodies;

const float MAX_PHYSICS_TIME = 500.0f;

glm::dvec3 orbitalVelocity(size_t parent, size_t orbiter);

glm::dvec3 gravitationalForce(const GravityBody& a, const GravityBody& b);
void updateBodies(glm::float64 deltaTime, std::vector<GravityBody>& bodies);
void physicsLoop(GLFWwindow* window);