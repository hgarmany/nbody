#include "physics.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <omp.h>

std::vector<std::shared_ptr<GravityBody>> bodies, frameBodies;
std::ofstream physicsLog1, physicsLog2;

uint8_t targetRotation = 0;

Camera camera;
Camera pipCam(
	glm::dvec3(0, 1e6, 0),
	glm::dvec3(0, -1, 0),
	glm::dvec3(1, 0, 0));

std::atomic<bool> running(true);
std::condition_variable physicsCV;
bool hasPhysics = false;
bool doTrails = true;
bool physicsUpdated = false;
double frameTime = 0.0;
double elapsedTime = 0.0;
double timeStep = 1e5;
size_t maxTrailLength = 2500;

void mergeNearBodies() {
	for (int i = 0; i < bodies.size(); i++) {
		for (int j = i + 1; j < bodies.size(); j++) {
			// bodies within a medium distance are merged into one body
			double minDist = fmax(bodies[i]->radius, bodies[j]->radius) / 5.0;
			if (glm::distance(bodies[i]->position, bodies[j]->position) < minDist) {
				std::shared_ptr<GravityBody> a = bodies[i];
				std::shared_ptr<GravityBody> b = bodies[j];

				// mass relative to the 2-body system
				double m1 = a->mass / (a->mass + b->mass);
				double m2 = b->mass / (a->mass + b->mass);
				
				// set mass-centered ballistic properties
				glm::dvec3 center = a->position * m1 + b->position * m2;
				glm::dvec3 vel = a->velocity * m1 + b->velocity * m2;
				a->position = center;
				a->velocity = vel;
				a->mass += b->mass;

				// resize volume
				double r1 = a->radius;
				double r2 = b->radius;
				a->radius = cbrt(r1 * r1 * r1 + r2 * r2 * r2);
				a->scale = glm::dvec3(a->radius);
				bodies.erase(bodies.begin() + j);
				j--;
			}
		}
	}
}

glm::dmat4 relativeRotationalMatrix(std::vector<std::shared_ptr<GravityBody>>& list, size_t subjectIndex, size_t referenceIndex, bool detranslate) {
	std::shared_ptr<GravityBody> subject = list[subjectIndex];
	std::shared_ptr<GravityBody> reference = list[referenceIndex];

	// develops a rotational matrix to transform subject coordinates to a reference frame 
	// in which the reference body and its parent both lie along the x axis
	glm::dmat4 rotate(1.0);
	if (reference != list[subject->parentIndex]) {
		size_t aParentIndex = reference->parentIndex;
		glm::dvec3 a = glm::normalize(reference->position - list[aParentIndex]->position);
		glm::dvec3 b(1.0, 0.0, 0.0);
		glm::dvec3 n = glm::cross(a, reference->velocity);

		double angle = acos(glm::dot(a, b));
		if (glm::dot(n, glm::cross(a, b)) < 0)
			angle *= -1;

		// shaders will detranslate spatial adjustments
		if (detranslate)
			rotate = glm::rotate(rotate, -angle, n);
		else
			rotate = glm::rotate(rotate, angle, n);
	}
	return rotate;
}

glm::dvec3 orbitalVelocity(size_t parent, size_t orbiter) {
	glm::dvec3 gravitation = bodies[parent]->position - bodies[orbiter]->position;
	double distance = glm::length(gravitation);
	double magnitude = sqrt(G * bodies[parent]->mass / distance);
	return magnitude * glm::normalize(glm::cross(glm::dvec3(0, 1, 0), gravitation));
}

glm::vec3 eccentricityVector(size_t parent, size_t orbiter) {
	glm::dvec3 position = bodies[orbiter]->position - bodies[parent]->position;
	glm::dvec3 velocity = bodies[orbiter]->velocity - bodies[parent]->velocity;

	glm::dvec3 orbitalMomentum = glm::cross(position, velocity);
	return glm::cross(velocity, orbitalMomentum) / (G * bodies[parent]->mass) - position / glm::length(position);
}

double semiMajorAxis(size_t parent, size_t orbiter) {
	glm::float64 distance = glm::distance(bodies[orbiter]->position, bodies[parent]->position);
	glm::float64 speed = glm::distance(bodies[orbiter]->velocity, bodies[parent]->velocity);
	return 1.0 / (2.0 / distance - speed * speed / (G * bodies[parent]->mass));
}

void gravitationalForce(std::shared_ptr<GravityBody> a, std::shared_ptr<GravityBody> b) {
	glm::dvec3 displacement = b->position - a->position;
	glm::float64 distance = glm::length(displacement);
	glm::dvec3 direction = glm::normalize(displacement);
	glm::dvec3 fieldLine = (G * direction) / (distance * distance);
	glm::dvec3 accelerationA = b->mass * fieldLine;

	if (b->gravityType == OBLATE_SPHERE) {
		// oblate perturbations on a by b, using MacCullagh's formula
		glm::dvec3 axisOfRotation = b->rotQuat * glm::dvec3(0.0, 1.0, 0.0);
		double cosTheta = glm::dot(direction, axisOfRotation);
		double sinTheta = sqrt(1 - cosTheta * cosTheta);
		double radiusOverDistance = b->radius / distance;
		accelerationA *= 1 - 3.0 * b->j2 * radiusOverDistance * radiusOverDistance * (3.0 * sinTheta * sinTheta - 1.0);

		// torque
		double torqueMagnitude = 3.0 * G * a->mass * b->mass * b->j2 * radiusOverDistance * radiusOverDistance / distance * sinTheta * cosTheta;

		b->torque += torqueMagnitude * glm::normalize(glm::cross(direction, axisOfRotation));
	}

	glm::dvec3 accelerationB = -a->mass * fieldLine;

	if (a->gravityType == OBLATE_SPHERE) {
		// oblate perturbations on b by a, using MacCullagh's formula
		glm::dvec3 axisOfRotation = a->rotQuat * glm::dvec3(0.0, 1.0, 0.0);
		double cosTheta = glm::dot(direction, axisOfRotation);
		double sinTheta = sqrt(1 - cosTheta * cosTheta);
		double radiusOverDistance = a->radius / distance;
		accelerationB *= 1 - 3.0 * a->j2 * radiusOverDistance * radiusOverDistance * (3.0 * sinTheta * sinTheta - 1.0);

		// torque
		double torqueMagnitude = 3.0 * G * a->mass * b->mass * a->j2 * radiusOverDistance * radiusOverDistance / distance * abs(sinTheta * cosTheta);

		a->torque += torqueMagnitude * glm::normalize(glm::cross(axisOfRotation, direction));
	}

	a->acceleration += accelerationA;
	b->acceleration += accelerationB;
}

void updateBodies(glm::float64 deltaTime, std::vector<std::shared_ptr<GravityBody>>& bodies) {
	glm::float64 fullDt = timeStep * deltaTime;
	glm::float64 halfDt = fullDt * 0.5;

	// Update velocities and positions by half-step, clear accelerations
	for (std::shared_ptr<GravityBody> body : bodies) {
		body->velocity += body->acceleration * halfDt;
		body->angularMomentum += body->torque * halfDt;

		body->prevPosition = body->position;
		body->position += body->velocity * fullDt;
		
		body->rotateRK4(fullDt);
		
		body->acceleration = body->torque = glm::dvec3(0.0);
	}

	// Compute forces between particles
	for (size_t i = 0; i < bodies.size(); ++i) {
		for (size_t j = i + 1; j < bodies.size(); ++j) {
			gravitationalForce(bodies[i], bodies[j]);
		}
	}

	// Update velocities to full-step using the new accelerations
	for (std::shared_ptr<GravityBody> body : bodies) {
		//body.torque = glm::dvec3(0.0);
		body->velocity += body->acceleration * halfDt;
		body->angularMomentum += body->torque * halfDt;
	}

	elapsedTime += fullDt;
}

void tracePath(size_t parent, size_t orbiter) {
	bool doLoop = true;

	// remove any trail points behind the body's position
	while (doLoop && bodies[orbiter]->trail->size() > 1 &&
		parent == bodies[orbiter]->parentIndex) {
		glm::dvec3 a = glm::normalize(bodies[orbiter]->trail->front());
		glm::dvec3 b = glm::normalize(bodies[orbiter]->trail->back());

		glm::dvec3 n = glm::cross(b, bodies[orbiter]->position - bodies[parent]->position);

		// get angle between the last point added to the trail and the start of the trail
		double angle = acos(glm::dot(a, b));
		if (glm::dot(n, glm::cross(a, b)) < 0)
			angle *= -1;

		// get angle between the body's current position and the start of the trail
		b = glm::normalize(bodies[orbiter]->position - bodies[parent]->position);
		double angle2 = acos(glm::dot(a, b));
		if (glm::dot(n, glm::cross(a, b)) < 0)
			angle2 *= -1;

		if (angle < 0 && angle2 >= 0 || std::isnan(angle) || std::isnan(angle2))
			bodies[orbiter]->trail->pop();
		else
			doLoop = false;
	}

	glm::dvec3 relativePosition =
		glm::dmat3(relativeRotationalMatrix(bodies, orbiter, parent)) *
		glm::dvec3(bodies[orbiter]->position - bodies[parent]->position);

	if (bodies[orbiter]->trail->size() == 0 || glm::distance(bodies[orbiter]->trail->back(), relativePosition) * 2 > bodies[orbiter]->radius)
		bodies[orbiter]->trail->push(relativePosition);
}

void ellipticalPath(size_t parent, size_t orbiter) {
	glm::float64 m0 = bodies[parent]->mass;
	glm::float64 m1 = bodies[orbiter]->mass;
	glm::dvec3 r0 = bodies[parent]->position;
	glm::dvec3 r1 = bodies[orbiter]->position;
	glm::dvec3 centerOfMass = (m0 * r0 + m1 * r1) / (m0 + m1);
	glm::float64 gravParam = G * (m0 + m1);

	glm::dvec3 position = r1 - r0;
	glm::dvec3 velocity = bodies[orbiter]->velocity - bodies[parent]->velocity;

	glm::dvec3 orbitalMomentum = glm::cross(position, velocity);
	glm::dvec3 h_hat = glm::normalize(orbitalMomentum);
	glm::dvec3 eccVector = glm::cross(velocity, orbitalMomentum) / gravParam - position / glm::length(position);
	glm::float64 eccentricity = glm::length(eccVector);
	glm::dvec3 e_hat = glm::normalize(eccVector);
	
	glm::float64 distance = glm::length(position);
	glm::float64 speed = glm::length(velocity);

	glm::float64 semiMajorAxisLen = 1.0 / (2.0 / distance - speed * speed / gravParam);

	bodies[orbiter]->trail->queue.clear();
	double perimeter = ellipsePerimeter(semiMajorAxisLen, (float)eccentricity);
	//double thetaStep = std::min(pi * bodies[orbiter]->radius / perimeter, 0.002 * pi);
	double thetaStep = 0.002 * pi;

	for (double theta = 0; theta <= 2 * pi; theta += thetaStep) {
		// polar radius
		double r = semiMajorAxisLen * (1 - eccentricity * eccentricity) / (1 + eccentricity * cos(theta));
		glm::dvec3 trailPoint = e_hat * (r * cos(theta)) + glm::cross(h_hat, e_hat) * (r * sin(theta));
		bodies[orbiter]->trail->push(trailPoint);
	}
	bodies[orbiter]->trail->push(bodies[orbiter]->trail->front());
}

void updateTrails(std::vector<std::shared_ptr<GravityBody>> bodies) {
	int num_threads = 4;
	omp_set_num_threads(num_threads);

	#pragma omp parallel 
	for (size_t i = 0; i < bodies.size(); i++) {
		std::shared_ptr<GravityBody> body = bodies[i];
		Trail* trail = body->trail;
		if (trail) {
			size_t parentIndex = body->trail->parentIndex;

			// trail relative to parent body
			if (parentIndex != -1) {
				ellipticalPath(parentIndex, i);
			}

			// trail relative to world space
			else {
				trail->push(body->position);
				while (trail->size() > maxTrailLength) {
					trail->pop();
				}
			}
		}
	}
}

void physicsLoop() {
	// log file test
	std::ofstream timeLog = std::ofstream("time.txt", std::ios::out);
	physicsLog1 = std::ofstream("log1.txt", std::ios::out);
	physicsLog2 = std::ofstream("log2.txt", std::ios::out);
	glm::dvec3 up = bodies[0]->rotQuat * glm::dvec3(0.0, 1.0, 0.0);
	glm::dvec3 referenceDirection = glm::normalize(glm::dvec3(1.0, 0.0, 0.0)); // fixed vector
	glm::dvec3 cross = glm::normalize(glm::cross(up, referenceDirection));
	double totalTimeElapsed = 0.0;

	double lastLoopTime = glfwGetTime();
	while (running) {
		double currentTime = glfwGetTime();
		glm::float64 deltaTime = currentTime - lastLoopTime;
		lastLoopTime = currentTime;

		if (deltaTime * timeStep < MAX_PHYSICS_TIME) {
			if (hasPhysics) {
				frameTime = deltaTime * timeStep;
				totalTimeElapsed += deltaTime * timeStep;
				
				updateBodies(deltaTime, bodies);

				if (totalTimeElapsed / 7889231 > 1) {
					physicsLog1 << std::setprecision(10) << glm::distance(bodies[1]->position, bodies[0]->position) << std::endl;
					physicsLog2 << std::setprecision(10) << glm::distance(bodies[2]->position, bodies[0]->position) << std::endl;
					totalTimeElapsed -= 7889231;
				}
			}

			// data is ready for renderer to access
			physicsUpdated = true;
			physicsCV.notify_one();
		}
		else
			printf("discarded physics frame: %.2f ms\n", deltaTime * 1000);
	}

	timeLog.close();
	physicsLog1.close();
	physicsLog2.close();
}