#pragma once

#include "entity.h"
#include <deque>

class GravityBody : public Entity {
public:
	std::deque<glm::dvec3>* trail;
	glm::vec3 trailColor;
	glm::float64 mass;
	glm::float64 radius;

	GravityBody(glm::float64 mass) {
		trail = nullptr;
		trailColor = glm::vec3(0.0f);
		this->mass = mass;
		radius = 0.0;
		position = glm::dvec3(0.0);
		velocity = glm::dvec3(0.0);
		acceleration = glm::dvec3(0.0);
		
		modelMatrix = glm::dmat4(0.0);
	}
};