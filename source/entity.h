#pragma once

#include "model.h"
#include "shader.h"
#include "surface.h"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

class Entity {
public:
	static Entity skybox;

	glm::dmat4 modelMatrix;
	Surface surface;
	glm::dvec3 position;
	glm::dvec3 prevPosition;
	glm::dvec3 velocity;
	glm::dvec3 rotVelocity;
	glm::dvec3 acceleration;
	glm::dvec3 orientation;
	glm::dvec3 scale;
	size_t modelIndex;

	Entity() {
		modelMatrix = glm::dmat4(1.0f);

		position = glm::dvec3(0.0f);
		prevPosition = position;
		velocity = glm::dvec3(0.0f);
		rotVelocity = glm::dvec3(0.0f);
		acceleration = glm::dvec3(0.0f);
		orientation = glm::dvec3(0.0f, 0.0f, 0.0f);
		scale = glm::dvec3(1.0f);

		surface.material = glm::vec4(0.1f, 1.0f, 1.0f, 0.0f);
		surface.color = glm::vec3(1.0f);
		surface.texture = 0;

		modelIndex = -1;
	}

	glm::dquat getRotationQuat();
	glm::dmat4 updateMatrix();
	void draw(Shader shader, uint8_t mode);

	virtual ~Entity() {}
};