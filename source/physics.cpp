#include "physics.h"
#include "controls.h"
#include <vector>

std::vector<GravityBody> bodies;
std::vector<GravityBody> frameBodies;

std::atomic<bool> running(true);
std::condition_variable physicsCV;
bool physicsUpdated = false;
int physicsFrames = 0, lastPhysicsFrames = 0;

glm::dvec3 gravitationalForce(const GravityBody& a, const GravityBody& b) {
    glm::dvec3 direction = b.position - a.position;
    glm::float64 distance = glm::length(direction);
    glm::float64 forceMagnitude = (G * a.mass * b.mass) / (distance * distance);
    return forceMagnitude * glm::normalize(direction);
}

void updateBodies(glm::float64 deltaTime, std::vector<GravityBody>& bodies) {
    glm::float64 halfDt = deltaTime * timeStep * 0.5;
    glm::float64 fullDt = deltaTime * timeStep;

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
}

void physicsLoop(GLFWwindow* window) {
    double lastLoopTime = glfwGetTime();
    while (running) {

        double currentTime = glfwGetTime();
        glm::float64 deltaTime = currentTime - lastLoopTime;
        lastLoopTime = currentTime;

        if (hasPhysics)
            updateBodies(deltaTime, bodies);

        // manual control: adjust earth axial tilt and time of day
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            bodies[3].orientation.y += deltaTime;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            bodies[3].orientation.y -= deltaTime;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            bodies[3].orientation.x += deltaTime;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            bodies[3].orientation.x -= deltaTime;

        // data is ready for renderer to access
        physicsUpdated = true;
        physicsFrames++;
        physicsCV.notify_one();
    }
}