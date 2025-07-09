#pragma once

#include "model.h"
#include "shader.h"
#include "surface.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/quaternion.hpp>
#include <memory>

class Entity {
public:
	static std::shared_ptr<Entity> skybox;

	glm::dmat4 modelMatrix;
	glm::dquat rotQuat; // quaternion representation of orientation
	glm::dvec3 position, prevPosition, velocity, acceleration;
	glm::dvec3 scale;
	size_t modelIndex;
	std::shared_ptr<Entity> root;
	Surface surface;

	Entity() {
		modelMatrix = glm::dmat4(1.0f);
		rotQuat = glm::dquat(1.0, 0.0, 0.0, 0.0);

		position = prevPosition = velocity = acceleration = glm::dvec3(0.0f);
		scale = glm::dvec3(1.0f);

		surface = Surface(glm::vec4(0.1f, 1.0f, 1.0f, 0.0f));
		modelIndex = -1;

		root = nullptr;
	}

	//glm::dvec3 updateRotationMatrix();
	glm::dmat4 updateMatrix(bool doScale = true);
	void draw(Shader& shader, uint8_t mode, glm::dmat4 rootMatrix = glm::dmat4(1.0));

	virtual ~Entity() {}
};

extern std::vector<std::shared_ptr<Entity>> entities, frameEntities;