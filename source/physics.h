#pragma once

#include <condition_variable>
#include <thread>
#include <chrono>
#include "camera.h"
#include "gravitybody.h"

using Clock = std::chrono::high_resolution_clock;

extern Camera camera, pipCam;
extern std::atomic<bool> running;
extern std::condition_variable physicsDone, physicsStart;
extern std::mutex physicsMutex;
extern bool hasPhysics, doTrails;
extern double elapsedTime, timeStep, frameTime;
extern uint8_t targetRotation;

const float MAX_PHYSICS_TIME = 1000.0f;

glm::dvec3 orbitalVelocity(size_t parent, size_t orbiter);

void updateBodies(glm::float64 deltaTime, std::vector<GravityBody>& bodies);
void updateTrails(std::vector<std::shared_ptr<GravityBody>>& bodies);
glm::dmat4 relativeRotationalMatrix(std::vector<std::shared_ptr<GravityBody>>& list, 
	const std::shared_ptr<GravityBody>& subject, const std::shared_ptr<GravityBody>& reference, bool detranslate = false);
void initLoggers();
void physicsLoop();