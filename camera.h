#pragma once

#include <glm/gtc/quaternion.hpp>
#include "entity.h"
#include "util.h"

static const float SENSITIVITY = 0.1f * pi / 180.0f; // control on mouse sensitivity

class Camera {
public:
	glm::dvec3 position = glm::dvec3(0.0);
	glm::dvec3 velocity = glm::dvec3(0.0);
	glm::dvec3 up = glm::dvec3(0.0, 1.0, 0.0);
	glm::dvec3 direction = glm::dvec3(-1.0, 0.0, 0.0);
	glm::dvec3 right = glm::cross(direction, up);

	Camera() {
	}

	glm::mat4 viewMatrix() {
		return glm::dmat4(
			right.x, up.x, -direction.x, 0.0,
			right.y, up.y, -direction.y, 0.0,
			right.z, up.z, -direction.z, 0.0,
			-glm::dot(right, position), -glm::dot(up, position), glm::dot(direction, position), 1.0
		);
	}

	void watchFrom(Entity& observer)	{
		position = observer.position + observer.scale * glm::normalize(observer.velocity);
		direction = glm::normalize(observer.velocity);
		up = glm::dvec3(0.0, 1.0, 0.0);
		right = glm::cross(direction, up);
	}

	void setOrientation(glm::float64 pitch, glm::float64 yaw, glm::float64 roll) {
		glm::dquat yawQuat = glm::angleAxis(yaw * SENSITIVITY, up);
		direction = yawQuat * direction;
		right = yawQuat * right;

		glm::dquat pitchQuat = glm::angleAxis(pitch * SENSITIVITY, right);
		direction = pitchQuat * direction;
		up = pitchQuat * up;

		glm::dquat rollQuat = glm::angleAxis(roll, direction);
		up = rollQuat * up;
		right = rollQuat * right;
	}
};