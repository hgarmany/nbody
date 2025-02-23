#include "physics.h"
#include <vector>

std::vector<GravityBody> bodies;
std::vector<GravityBody> frameBodies;

Camera camera;
Camera pipCam(
	glm::dvec3(0, 1e6, 0),
	glm::dvec3(0, -1, 0),
	glm::dvec3(1, 0, 0));

std::atomic<bool> running(true);
std::condition_variable physicsCV;
bool hasPhysics = false;
bool physicsUpdated = false;
double elapsedTime = 0.0;
double timeStep = 1e5;

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

glm::dvec3 orbitalVelocity(size_t parent, size_t orbiter) {
	glm::dvec3 gravitation = bodies[parent].position - bodies[orbiter].position;
	double distance = glm::length(gravitation);
	double magnitude = sqrt(G * bodies[parent].mass / distance);
	return magnitude * glm::normalize(glm::cross(glm::dvec3(0, 1, 0), gravitation));
}

glm::dvec3 gravitationalForce(const GravityBody& a, const GravityBody& b) {
	glm::dvec3 direction = b.position - a.position;
	glm::float64 distance = glm::length(direction);
	glm::float64 forceMagnitude = (G * a.mass * b.mass) / (distance * distance);
	return forceMagnitude * glm::normalize(direction);
}

void updateBodies(glm::float64 deltaTime, std::vector<GravityBody>& bodies) {
	glm::float64 fullDt = timeStep * deltaTime;
	glm::float64 halfDt = fullDt * 0.5;

	// Update velocities and positions by half-step, clear accelerations
	for (GravityBody& body : bodies) {
		body.velocity += body.acceleration * halfDt;
		body.position += body.velocity * fullDt;
		body.acceleration = glm::dvec3(0.0);
	}

	// Compute forces between particles
	for (size_t i = 0; i < bodies.size(); ++i) {
		for (size_t j = i + 1; j < bodies.size(); ++j) {
			glm::dvec3 force = gravitationalForce(bodies[i], bodies[j]);
			bodies[i].acceleration += force / bodies[i].mass;
			bodies[j].acceleration -= force / bodies[j].mass;
		}
	}

	// Update velocities to full-step using the new accelerations
	for (GravityBody& body : bodies) {
		body.velocity += body.acceleration * halfDt;
		body.orientation += body.rotVelocity * timeStep * deltaTime;
	}

	elapsedTime += deltaTime * timeStep;
}

void physicsLoop(GLFWwindow* window) {
	double lastLoopTime = glfwGetTime();
	while (running) {
		double currentTime = glfwGetTime();
		glm::float64 deltaTime = currentTime - lastLoopTime;
		lastLoopTime = currentTime;

		if (deltaTime * timeStep < MAX_PHYSICS_TIME) {
			if (hasPhysics)
				updateBodies(deltaTime, bodies);

			if (camera.atIndex != -1) {
				GravityBody& body = bodies[camera.atIndex];
				// spin the subject body around
				if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
					body.orientation.y += deltaTime;
				if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
					body.orientation.y -= deltaTime;
				if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
					body.orientation.x += deltaTime;
				if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
					body.orientation.x -= deltaTime;
			}
			if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
				bodies[bodies.size() - 1].velocity = glm::dvec3(0.0);

			// data is ready for renderer to access
			physicsUpdated = true;
			physicsCV.notify_one();
		}
		else
			printf("discarded physics frame: %.2f ms\n", deltaTime * 1000);
	}
}