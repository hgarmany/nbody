#pragma once

#include "model.h"
#include "shader.h"
#include "surface.h"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

class Entity {
public:
	std::shared_ptr<Model> model;

	glm::dmat4 modelMatrix;
	glm::dvec3 position;
	glm::dvec3 prevPosition;
	glm::dvec3 velocity;
	glm::dvec3 acceleration;
	glm::dvec3 orientation;
	glm::dvec3 scale;
	Surface surface;

	Entity() {
		modelMatrix = glm::dmat4(1.0f);

		position = glm::dvec3(0.0f);
		prevPosition = position;
		velocity = glm::dvec3(0.0f);
		acceleration = glm::dvec3(0.0f);
		orientation = glm::dvec3(0.0f, 0.0f, 0.0f);
		scale = glm::dvec3(1.0f);

		surface.material = glm::vec4(0.1f, 1.0f, 1.0f, 0.0f);
		surface.color = glm::vec3(1.0f);
		surface.texture = -1;
	}

	glm::dquat getRotationQuat();
	glm::dmat4 updateMatrix();
	void draw(Shader shader);

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

	void setModel(std::shared_ptr<Model> model) {
		entity->model = model;
	}

	void setMotion(glm::dvec3 position, glm::dvec3 velocity = glm::dvec3(0.0f)) {
		entity->position = position;
		entity->prevPosition = position;
		entity->velocity = velocity;
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