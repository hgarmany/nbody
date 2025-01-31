#pragma once

#include "model.h"
#include "shader.h"
#include "surface.h"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

class Entity {
public:
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
		surface.texture = -1;

		modelIndex = -1;
	}

	glm::dquat getRotationQuat();
	glm::dmat4 updateMatrix();
	void draw(Shader shader, uint8_t mode);

	virtual ~Entity() {}
};

class EntityBuilder {
protected:
	std::shared_ptr<Entity> entity;
public:
	EntityBuilder() {
	}

	void init() {
		entity = std::make_shared<Entity>();
	}

	void setModel(size_t modelIndex) {
		entity->modelIndex = modelIndex;
	}

	void setMotion(
		glm::dvec3 position, 
		glm::dvec3 velocity = glm::dvec3(0.0f), 
		glm::dvec3 rotVelocity = glm::dvec3(0.0f)
	) {
		entity->position = position;
		entity->prevPosition = position;
		entity->velocity = velocity;
		entity->rotVelocity = rotVelocity;
	}

	void setOrientation(glm::dvec3 orientation) {
		entity->orientation = orientation;
	}

	void setScale(glm::dvec3 scale) {
		entity->scale = scale;
	}

	void setSurface(Surface surface) {
		entity->surface = surface;
	}

	Entity get() {
		entity->updateMatrix();
		Entity temp = *entity;
		entity.reset();
		return temp;
	}
};