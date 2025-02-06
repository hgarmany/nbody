#pragma once

#include <glm/gtc/quaternion.hpp>
#include "entity.h"
#include "util.h"

static const float SENSITIVITY = 0.002f; // control on mouse sensitivity

class Camera {
public:
	glm::dvec3 position;
	glm::dvec3 direction;
	glm::dvec3 up;
	glm::dvec3 velocity;
	glm::dvec3 right;

	Camera(
		glm::dvec3 position = glm::dvec3(0.0), 
		glm::dvec3 direction = glm::dvec3(-1.0, 0.0, 0.0), 
		glm::dvec3 up = glm::dvec3(0.0, 1.0, 0.0)
	) {
		this->position = position;
		this->direction = direction;
		this->up = up;

		velocity = glm::dvec3(0.0);
		right = glm::cross(direction, up);
	}

	glm::dmat4 viewMatrix() {
		return glm::dmat4(
			right.x, up.x, -direction.x, 0.0,
			right.y, up.y, -direction.y, 0.0,
			right.z, up.z, -direction.z, 0.0,
			-glm::dot(right, position), -glm::dot(up, position), glm::dot(direction, position), 1.0
		);
	}

	// place camera in front of observer, facing in the direction of movement
	void watchFrom(Entity& observer)	{
		position = observer.position + observer.scale * glm::normalize(observer.velocity);
		direction = glm::normalize(observer.velocity);
		up = glm::dvec3(0.0, 1.0, 0.0);
		right = glm::cross(direction, up);
	}

	// perform local relative rotations in order: yaw, pitch, roll
	void setOrientation(glm::float64 pitch, glm::float64 yaw, glm::float64 roll) {
		glm::dquat yawQuat = glm::angleAxis(yaw * SENSITIVITY, up);
		direction = yawQuat * direction;
		right = yawQuat * right;

		glm::dquat pitchQuat = glm::angleAxis(pitch * SENSITIVITY, right);
		direction = pitchQuat * direction;
		up = pitchQuat * up;

		glm::dquat rollQuat = glm::angleAxis(roll * SENSITIVITY, direction);
		up = rollQuat * up;
		right = rollQuat * right;
	}
};