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

const float MAX_PHYSICS_TIME = 3600.0f;

glm::dvec3 orbitalVelocity(size_t parent, size_t orbiter);

void updateTrails(context& bodies);
glm::dmat4 relativeRotationalMatrix(context& list, 
	const std::shared_ptr<GravityBody>& subject, const std::shared_ptr<GravityBody>& reference, bool detranslate = false);
void initLoggers();
void physicsLoop();