#pragma once

#include "gravitybody.h"

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
		glm::dvec3 velocity = glm::dvec3(0.0f)
	) {
		entity->position = position;
		entity->prevPosition = position;
		entity->velocity = velocity;
	}

	void setRotation(
		glm::dvec3 orientation,
		glm::dvec3 rotVelocity = glm::dvec3(0.0f)
	) {
		entity->rotQuat = glm::dquat(orientation);
		if (auto body = std::dynamic_pointer_cast<GravityBody>(entity)) {
			body->initI();
			body->angularMomentum = body->rotQuat * (body->momentOfInertia * rotVelocity);
			if (body->gravityType == OBLATE_SPHERE)
				body->initJ2();
		}
	}

	void setScale(glm::dvec3 scale) {
		entity->scale = scale;
		if (auto body = std::dynamic_pointer_cast<GravityBody>(entity)) {
			body->gravityType = CUBOID;
			body->radius = std::min({ scale.x, scale.y, scale.z });
		}
	}

	void setSurface(Surface surface) {
		entity->surface = surface;
	}

	void setRoot(std::shared_ptr<Entity> root) {
		entity->root = root;
	}

	std::shared_ptr<Entity> get() {
		entity->updateMatrix();
		return entity;
	}

	void buildSky(size_t modelIndex);
};


class GravityBodyBuilder : public EntityBuilder {
public:
	GravityBodyBuilder() {
	}

	void init(double mass = DBL_MIN) {
		if (entity)
			entity.reset();
		entity = std::make_shared<GravityBody>(mass);
	}

	void init(double mass, Orbit orbit, size_t parentIndex) {
		entity = std::make_shared<GravityBody>(mass, orbit, parentIndex);
	}

	void setRadius(float radius, float oblateness = 0.0f) {
		if (auto body = std::dynamic_pointer_cast<GravityBody>(entity)) {
			body->radius = radius;
			body->scale = glm::dvec3(radius);
			body->oblateness = oblateness;
			if (oblateness != 0.0f) {
				body->gravityType = OBLATE_SPHERE;
			}
		}
	}

	void addTrail(glm::vec3 color = glm::vec3(1.0f), size_t parentIndex = -1) {
		if (auto body = std::dynamic_pointer_cast<GravityBody>(entity)) {
			if (parentIndex == -1)
				body->trail = new Trail(color, body->parentIndex);
			else
				body->trail = new Trail(color, parentIndex);
		}
	}

	std::shared_ptr<GravityBody> get() {
		if (auto body = std::dynamic_pointer_cast<GravityBody>(entity)) {
			body->updateMatrix();
			return body;
		}
		return nullptr;
	}

	void buildSolarSystem(size_t modelIndex);
};