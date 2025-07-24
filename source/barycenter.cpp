#include "barycenter.h"

TwoBodyBarycenter::TwoBodyBarycenter(size_t a, size_t b) {
	if (bodies.size() < a || bodies.size() < b) {
		primary = -1;
		secondary = -1;
	}
	else if (bodies[a]->mass >= bodies[b]->mass) {
		primary = a;
		secondary = b;

		bodies[a]->barycenter = this;
	}
	else {
		primary = b;
		secondary = a;

		bodies[b]->barycenter = this;
	}

	glm::vec3 newColor = bodies[primary]->trail->color;

	if (primary % 3 == 0)
		newColor.r = newColor.r > 0.5f ? newColor.r - 0.5f : newColor.r + 0.5f;
	if (primary % 3 == 1)
		newColor.g = newColor.g > 0.5f ? newColor.g - 0.5f : newColor.g + 0.5f;
	if (primary % 3 == 2)
		newColor.b = newColor.b > 0.5f ? newColor.b - 0.5f : newColor.b + 0.5f;

	primaryOrbit = new Trail(newColor, primary);
}

glm::float64 TwoBodyBarycenter::mass() {
	return frameBodies[primary]->mass + frameBodies[secondary]->mass;
}

glm::dvec3 TwoBodyBarycenter::position() {
	return (frameBodies[primary]->mass * frameBodies[primary]->position +
		frameBodies[secondary]->mass * frameBodies[secondary]->position) /
		(frameBodies[primary]->mass + frameBodies[secondary]->mass);
}

glm::dvec3 TwoBodyBarycenter::velocity() {
	return (frameBodies[primary]->mass * frameBodies[primary]->velocity +
		frameBodies[secondary]->mass * frameBodies[secondary]->velocity) /
		(frameBodies[primary]->mass + frameBodies[secondary]->mass);
}

glm::float64 TwoBodyBarycenter::apparentMass(size_t observer) {
	glm::dvec3 com = (frameBodies[primary]->mass * frameBodies[primary]->position +
		frameBodies[secondary]->mass * frameBodies[secondary]->position) /
		(frameBodies[primary]->mass + frameBodies[secondary]->mass);
	glm::float64 d1 = glm::distance(frameBodies[primary]->position, com);
	glm::float64 d2 = glm::distance(frameBodies[secondary]->position, com);

	if (observer == primary)
		return frameBodies[secondary]->mass * (d1 * d1) / ((d1 + d2) * (d1 + d2));
	if (observer == secondary)
		return frameBodies[primary]->mass * (d2 * d2) / ((d1 + d2) * (d1 + d2));
	return frameBodies[primary]->mass + frameBodies[secondary]->mass;
}

void TwoBodyBarycenter::positionOffset(glm::dvec3 offset) {
	bodies[primary]->position -= offset;
	bodies[secondary]->position -= offset;
}

void TwoBodyBarycenter::velocityOffset(glm::dvec3 offset) {
	bodies[primary]->velocity -= offset;
	bodies[secondary]->velocity -= offset;
}

ComplexBarycenter::ComplexBarycenter(size_t primary, size_t secondary) {
	this->primary = primary;
	this->secondaries.push_back(secondary);

	bodies[primary]->barycenter = this;

	Trail* primaryTrail = bodies[primary]->trail;
	if (primaryTrail) {
		glm::vec3 newColor = bodies[primary]->trail->color;

		if (primary % 3 == 0)
			newColor.r = newColor.r > 0.5f ? newColor.r - 0.5f : newColor.r + 0.5f;
		if (primary % 3 == 1)
			newColor.g = newColor.g > 0.5f ? newColor.g - 0.5f : newColor.g + 0.5f;
		if (primary % 3 == 2)
			newColor.b = newColor.b > 0.5f ? newColor.b - 0.5f : newColor.b + 0.5f;

		primaryOrbit = new Trail(newColor, primary);
	}
	else {
		primaryOrbit = new Trail(glm::vec3(primary % 10 * 0.1f, (3 + primary % 10) * 0.1f, (7 + primary % 10) * 0.1f), primary);
	}
}

ComplexBarycenter::ComplexBarycenter(size_t primary, std::vector<size_t> secondaries) {
	this->primary = primary;
	this->secondaries = secondaries;

	bodies[primary]->barycenter = this;
	primaryOrbit = new Trail(bodies[primary]->trail->color, primary);
}

ComplexBarycenter::ComplexBarycenter(std::vector<size_t> indices) {
	glm::float64 highestMass = 0.0;
	size_t highestMassIndex = -1;
	for (size_t i : indices) {
		if (bodies[i]->mass > highestMass) {
			highestMass = bodies[i]->mass;
			highestMassIndex = i;
		}
	}
	primary = highestMassIndex;
	bodies.erase(bodies.begin() + highestMassIndex);
	secondaries = indices;

	bodies[primary]->barycenter = this;
	primaryOrbit = new Trail(bodies[primary]->trail->color, primary);
}

glm::float64 ComplexBarycenter::mass() {
	glm::float64 mass = frameBodies[primary]->mass;
	for (size_t secondary : secondaries)
		mass += frameBodies[secondary]->mass;
	return mass;
}

glm::dvec3 ComplexBarycenter::position() {
	glm::float64 mass = frameBodies[primary]->mass;
	glm::dvec3 massWeightedPos = frameBodies[primary]->mass * frameBodies[primary]->position;
	for (size_t secondary : secondaries) {
		mass += frameBodies[secondary]->mass;
		massWeightedPos += frameBodies[secondary]->mass * frameBodies[secondary]->position;
	}
	return massWeightedPos / mass;
}

glm::dvec3 ComplexBarycenter::velocity() {
	glm::float64 mass = frameBodies[primary]->mass;
	glm::dvec3 massWeightedVel = frameBodies[primary]->mass * frameBodies[primary]->velocity;
	for (size_t secondary : secondaries) {
		mass += frameBodies[secondary]->mass;
		massWeightedVel += frameBodies[secondary]->mass * frameBodies[secondary]->velocity;
	}
	return massWeightedVel / mass;
}

glm::float64 ComplexBarycenter::apparentMass(size_t observer) {
	glm::dvec3 netAcceleration(0.0);

	if (observer != primary) {
		glm::dvec3 r = frameBodies[primary]->position - frameBodies[observer]->position;
		glm::float64 d = glm::length(r);
		netAcceleration += frameBodies[primary]->mass * glm::normalize(r) / (d * d);
	}
	for (size_t secondary : secondaries) {
		if (observer != secondary) {
			glm::dvec3 r = frameBodies[secondary]->position - frameBodies[observer]->position;
			glm::float64 d = glm::length(r);
			netAcceleration += frameBodies[secondary]->mass * glm::normalize(r) / (d * d);
		}
	}

	glm::dvec3 com = position();
	glm::float64 comDistance = glm::distance(frameBodies[observer]->position, com);

	return glm::length(netAcceleration) * comDistance * comDistance;
}

void ComplexBarycenter::positionOffset(glm::dvec3 offset) {
	bodies[primary]->position -= offset;
	for (size_t secondary : secondaries)
		bodies[secondary]->position -= offset;
}

void ComplexBarycenter::velocityOffset(glm::dvec3 offset) {
	bodies[primary]->velocity -= offset;
	for (size_t secondary : secondaries)
		bodies[secondary]->velocity -= offset;
}