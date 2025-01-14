#include "physics.h"
#include <vector>

std::vector<GravityBody> bodies;

glm::dvec3 gravitationalForce(const GravityBody& a, const GravityBody& b) {
    glm::dvec3 direction = b.position - a.position;
    glm::float64 distance = glm::length(direction);
    glm::float64 forceMagnitude = (G * a.mass * b.mass) / (distance * distance);
    return forceMagnitude * glm::normalize(direction);
}

void updateBodies(glm::float64 deltaTime) {

    glm::float64 halfDt = deltaTime * TIME_STEP * 0.5;
    glm::float64 fullDt = deltaTime * TIME_STEP;

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
    }
}