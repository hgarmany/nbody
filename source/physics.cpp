#include "physics.h"
#include <vector>

std::vector<GravityBody> bodies;
std::vector<GravityBody> frameBodies;

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
double elapsedTime = 0.0;
double timeStep = 1e5;
size_t maxTrailLength = 2500;

void mergeNearBodies() {
	for (int i = 0; i < bodies.size(); i++) {
		for (int j = i + 1; j < bodies.size(); j++) {
			// bodies within a medium distance are merged into one body
			double minDist = fmax(bodies[i].radius, bodies[j].radius) / 5.0;
			if (glm::distance(bodies[i].position, bodies[j].position) < minDist) {
				GravityBody& a = bodies[i];
				GravityBody& b = bodies[j];

				// mass relative to the 2-body system
				double m1 = a.mass / (a.mass + b.mass);
				double m2 = b.mass / (a.mass + b.mass);
				
				// set mass-centered ballistic properties
				glm::dvec3 center = a.position * m1 + b.position * m2;
				glm::dvec3 vel = a.velocity * m1 + b.velocity * m2;
				a.position = center;
				a.velocity = vel;
				a.mass += b.mass;

				// resize volume
				double r1 = a.radius;
				double r2 = b.radius;
				a.radius = cbrt(r1 * r1 * r1 + r2 * r2 * r2);
				a.scale = glm::dvec3(a.radius);
				bodies.erase(bodies.begin() + j);
				j--;
			}
		}
	}
}

glm::dmat4 relativeRotationalMatrix(std::vector<GravityBody>& list, size_t subjectIndex, size_t referenceIndex, bool detranslate) {
	GravityBody* subject = &list[subjectIndex];
	GravityBody* reference = &list[referenceIndex];

	// develops a rotational matrix to transform subject coordinates to a reference frame 
	// in which the reference body and its parent both lie along the x axis
	glm::dmat4 rotate(1.0);
	if (reference != &list[subject->parentIndex]) {
		size_t aParentIndex = reference->parentIndex;
		glm::dvec3 a = glm::normalize(reference->position - list[aParentIndex].position);
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
	glm::dvec3 gravitation = bodies[parent].position - bodies[orbiter].position;
	double distance = glm::length(gravitation);
	double magnitude = sqrt(G * bodies[parent].mass / distance);
	return magnitude * glm::normalize(glm::cross(glm::dvec3(0, 1, 0), gravitation));
}

void gravitationalForce(GravityBody& a, GravityBody& b) {
	glm::dvec3 displacement = b.position - a.position;
	glm::float64 distance = glm::length(displacement);
	glm::dvec3 direction = glm::normalize(displacement);
	glm::dvec3 fieldLine = (G * direction) / (distance * distance);
	glm::dvec3 accelerationA = b.mass * fieldLine;

	if (b.gravityType == OBLATE_SPHERE) {
		// oblate perturbations on a by b, using MacCullagh's formula
		glm::dvec3 axisOfRotation = b.rotQuat * glm::dvec3(0.0, 1.0, 0.0);
		double sinTheta = glm::dot(direction, axisOfRotation);
		double radiusOverDistance = b.radius / distance;
		accelerationA *= 1 - 3.0 * b.j2 * radiusOverDistance * radiusOverDistance * (3.0 * sinTheta * sinTheta - 1.0);

		// torque
		double cosTheta = sqrt(1 - sinTheta * sinTheta);
		double torqueMagnitude = 3.0 * G * a.mass * b.mass * b.j2 * radiusOverDistance * radiusOverDistance / b.radius * sinTheta * cosTheta;

		b.torque += torqueMagnitude * glm::normalize(glm::cross(direction, axisOfRotation));
	}

	glm::dvec3 accelerationB = -a.mass * fieldLine;

	if (a.gravityType == OBLATE_SPHERE) {
		// oblate perturbations on b by a, using MacCullagh's formula
		glm::dvec3 axisOfRotation = a.rotQuat * glm::dvec3(0.0, 1.0, 0.0);
		double sinTheta = glm::dot(direction, axisOfRotation);
		double radiusOverDistance = a.radius / distance;
		accelerationB *= 1 - 3.0 * a.j2 * radiusOverDistance * radiusOverDistance * (3.0 * sinTheta * sinTheta - 1.0);

		// torque
		double cosTheta = sqrt(1 - sinTheta * sinTheta);
		double torqueMagnitude = 3.0 * G * a.mass * b.mass * a.j2 * radiusOverDistance * radiusOverDistance / a.radius * sinTheta * cosTheta;

		a.torque += torqueMagnitude * glm::normalize(glm::cross(axisOfRotation, direction));
	}

	a.acceleration += accelerationA;
	b.acceleration += accelerationB;
}

void updateBodies(glm::float64 deltaTime, std::vector<GravityBody>& bodies) {
	glm::float64 fullDt = timeStep * deltaTime;
	glm::float64 halfDt = fullDt * 0.5;

	// Update velocities and positions by half-step, clear accelerations
	for (GravityBody& body : bodies) {
		body.velocity += body.acceleration * halfDt;
		body.angularMomentum += body.torque * halfDt;

		body.position += body.velocity * fullDt;
		
		if (body.inertialTensor != glm::dvec3(0.0)) {
			// blackedout01: 3D rigid body physics
			glm::dmat3 inertiaMatrix(
				1.0 / body.inertialTensor.x, 0.0, 0.0,
				0.0, 1.0 / body.inertialTensor.y, 0.0,
				0.0, 0.0, 1.0 / body.inertialTensor.z
			);

			glm::dmat3 rotMatrix = glm::mat3_cast(body.rotQuat);
			glm::dvec3 omega = rotMatrix * inertiaMatrix * glm::transpose(rotMatrix) * body.angularMomentum;
			glm::dquat omegaQuat(0.0, omega.x, omega.y, omega.z);

			body.rotQuat += fullDt * omegaQuat * body.rotQuat;
			body.rotQuat = glm::normalize(body.rotQuat);
		}

		body.acceleration = body.torque = glm::dvec3(0.0);
	}

	// Compute forces between particles
	for (size_t i = 0; i < bodies.size(); ++i) {
		for (size_t j = i + 1; j < bodies.size(); ++j) {
			gravitationalForce(bodies[i], bodies[j]);
		}
	}

	// Update velocities to full-step using the new accelerations
	for (GravityBody& body : bodies) {
		body.velocity += body.acceleration * halfDt;
		body.angularMomentum += body.torque * halfDt;
	}

	elapsedTime += fullDt;
}

void updateTrails(std::vector<GravityBody>& bodies) {
	for (size_t i = 0; i < bodies.size(); i++) {
		GravityBody& body = bodies[i];
		Trail* trail = body.trail;
		if (trail) {
			size_t parentIndex = body.trail->parentIndex;

			// trail relative to parent body
			if (parentIndex != -1) {
				bool doLoop = true;

				// remove any trail points behind the body's position
				while (doLoop && trail->size() > 1 &&
					parentIndex == body.parentIndex) {
					glm::dvec3 a = glm::normalize(trail->front());
					glm::dvec3 b = glm::normalize(trail->back());

					glm::dvec3 n = glm::cross(b, body.position - bodies[parentIndex].position);

					// get angle between the last point added to the trail and the start of the trail
					double angle = acos(glm::dot(a, b));
					if (glm::dot(n, glm::cross(a, b)) < 0)
						angle *= -1;

					// get angle between the body's current position and the start of the trail
					b = glm::normalize(body.position - bodies[parentIndex].position);
					double angle2 = acos(glm::dot(a, b));
					if (glm::dot(n, glm::cross(a, b)) < 0)
						angle2 *= -1;

					if (angle < 0 && angle2 >= 0 || std::isnan(angle) || std::isnan(angle2))
						trail->pop();
					else
						doLoop = false;
				}

				glm::dvec3 relativePosition =
					glm::dmat3(relativeRotationalMatrix(bodies, i, parentIndex)) *
					glm::dvec3(body.position - bodies[body.parentIndex].position);

				trail->push(relativePosition);
			}

			// trail relative to world space
			else {
				trail->push(body.position);
				while (trail->size() > maxTrailLength) {
					trail->pop();
				}
			}
		}
	}
}

void physicsLoop() {
	double lastLoopTime = glfwGetTime();
	while (running) {
		double currentTime = glfwGetTime();
		glm::float64 deltaTime = currentTime - lastLoopTime;
		lastLoopTime = currentTime;

		if (deltaTime * timeStep < MAX_PHYSICS_TIME) {
			if (hasPhysics) {
				updateBodies(deltaTime, bodies);
			}

			/*if (targetRotation) {
				bodies[camera.atIndex].orientation.x += ((targetRotation & 0x01) - ((targetRotation & 0x02) >> 1)) * deltaTime;
				bodies[camera.atIndex].orientation.y += (((targetRotation & 0x04) >> 2) - ((targetRotation & 0x08) >> 3)) * deltaTime;
			}*/

			// data is ready for renderer to access
			physicsUpdated = true;
			physicsCV.notify_one();
		}
		else
			printf("discarded physics frame: %.2f ms\n", deltaTime * 1000);
	}
}