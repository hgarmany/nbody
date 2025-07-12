#pragma once

#include <gtc/quaternion.hpp>
#include "entity.h"

static const float SENSITIVITY = 0.002f; // control on mouse sensitivity

class Camera {
public:
	glm::dvec3 position, velocity;
	glm::dvec3 direction, up, right;
	size_t eyeIndex, atIndex;
	float FOV;
	float lockDistanceFactor;
	camera_mode mode;

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
		eyeIndex = atIndex = -1;
		FOV = glm::radians(45.0f);
		lockDistanceFactor = 5.0f;
		mode = FREE_CAM;
	}

	glm::dmat4 viewMatrix(glm::dvec3 position) {
		return glm::dmat4(
			right.x, up.x, -direction.x, 0.0,
			right.y, up.y, -direction.y, 0.0,
			right.z, up.z, -direction.z, 0.0,
			-glm::dot(right, position), -glm::dot(up, position), glm::dot(direction, position), 1.0
		);
	}

	glm::dmat4 viewMatrix() {
		return viewMatrix(position);
	}

	// watch one entity from the position of another
	void watchFrom(std::shared_ptr<Entity> at, std::shared_ptr<Entity> eye)	{
		position = eye->position;
		direction = glm::normalize(at->position - position);
		position += eye->scale.x * direction;
		right = glm::normalize(glm::cross(direction, up));
		up = glm::cross(right, direction);
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