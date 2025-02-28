#pragma once

#include <condition_variable>
#include <thread>
#include <vector>
#include "camera.h"
#include "gravitybody.h"

extern Camera camera, pipCam;
extern std::atomic<bool> running;
extern std::condition_variable physicsCV;
extern bool hasPhysics, physicsUpdated, doTrails;
extern double elapsedTime, timeStep;
extern uint8_t targetRotation;

extern std::vector<GravityBody> bodies, frameBodies;

const float MAX_PHYSICS_TIME = 500.0f;

glm::dvec3 orbitalVelocity(size_t parent, size_t orbiter);

void updateBodies(glm::float64 deltaTime, std::vector<GravityBody>& bodies);
void updateTrails(std::vector<GravityBody>& bodies);
glm::dmat4 relativeRotationalMatrix(std::vector<GravityBody>& list, size_t subjectIndex, size_t referenceIndex, bool detranslate = false);
void physicsLoop();