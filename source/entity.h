#pragma once

#include "model.h"
#include "shader.h"
#include "surface.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/quaternion.hpp>
#include <memory>

class Entity {
public:
	static Entity skybox;

	glm::dmat4 modelMatrix;
	glm::dquat rotQuat;
	Surface surface;
	glm::dvec3 position, prevPosition, velocity, acceleration;
	glm::dvec3 scale;
	size_t modelIndex;

	Entity() {
		modelMatrix = glm::dmat4(1.0f);
		rotQuat = glm::dquat(1.0, 0.0, 0.0, 0.0);

		position = prevPosition = velocity = acceleration = glm::dvec3(0.0f);
		scale = glm::dvec3(1.0f);

		surface = Surface(glm::vec4(0.1f, 1.0f, 1.0f, 0.0f));
		modelIndex = -1;
	}

	glm::dmat4 updateMatrix();
	void draw(Shader& shader, uint8_t mode);

	virtual ~Entity() {}
};