#pragma once

#include "entity.h"

class GravityBody : public Entity {
public:
	glm::float64 mass;
	glm::float64 radius;

	GravityBody(glm::float64 mass) {
		this->mass = mass;
		radius = 0.0;
		position = glm::dvec3(0.0);
		velocity = glm::dvec3(0.0);
		acceleration = glm::dvec3(0.0);
		
		modelMatrix = glm::dmat4(0.0);
	}
};

class GravityBodyBuilder : public EntityBuilder {
public:
	GravityBodyBuilder() {
	}

	void init(float mass) {
		entity = std::make_shared<GravityBody>(mass);
	}

	void setRadius(float radius) {
		if (auto body = std::dynamic_pointer_cast<GravityBody>(entity)) {
			body->radius = radius;
			body->scale = glm::dvec3(radius);
		}
	}

	GravityBody get() {
		if (auto body = std::dynamic_pointer_cast<GravityBody>(entity)) {
			body->updateMatrix();
			GravityBody temp = *body;
			entity.reset();
			return temp;
		}
		return GravityBody(0.0);
	}
};