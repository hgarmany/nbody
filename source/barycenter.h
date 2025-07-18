#pragma once

#include "gravitybody.h"

class Barycenter {
protected:
	size_t primary;
public:
	Trail* primaryOrbit;
	virtual glm::float64 mass() = 0;
	virtual glm::dvec3 position() = 0;
	virtual glm::dvec3 velocity() = 0;
	virtual glm::float64 apparentMass(size_t observer) = 0;
};

class TwoBodyBarycenter : public Barycenter {
protected:
	size_t secondary;
public:
	TwoBodyBarycenter(size_t a, size_t b) {
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

		primaryOrbit = new Trail(bodies[primary]->trail->color, primary);
	}

	glm::float64 mass() {
		return frameBodies[primary]->mass + frameBodies[secondary]->mass;
	}

	glm::dvec3 position() {
		return (frameBodies[primary]->mass * frameBodies[primary]->position +
			frameBodies[secondary]->mass * frameBodies[secondary]->position) /
			(frameBodies[primary]->mass + frameBodies[secondary]->mass);
	}

	glm::dvec3 velocity() {
		return (frameBodies[primary]->mass * frameBodies[primary]->velocity +
			frameBodies[secondary]->mass * frameBodies[secondary]->velocity) /
			(frameBodies[primary]->mass + frameBodies[secondary]->mass);
	}

	glm::float64 apparentMass(size_t observer) {
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
};

class ComplexBarycenter : public Barycenter {
protected:
	std::vector<size_t> secondaries;
public:
	ComplexBarycenter(size_t primary, std::vector<size_t> secondaries) {
		this->primary = primary;
		this->secondaries = secondaries;

		bodies[primary]->barycenter = this;
		primaryOrbit = new Trail(bodies[primary]->trail->color, primary);
	}

	ComplexBarycenter(std::vector<size_t> indices) {
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

	glm::float64 mass() {
		glm::float64 mass = bodies[primary]->mass;
		for (size_t secondary : secondaries)
			mass += bodies[secondary]->mass;
		return mass;
	}

	glm::dvec3 position() {
		glm::float64 mass = bodies[primary]->mass;
		glm::dvec3 massWeightedPos = bodies[primary]->mass * bodies[primary]->position;
		for (size_t secondary : secondaries) {
			mass += bodies[secondary]->mass;
			massWeightedPos += bodies[secondary]->mass * bodies[secondary]->position;
		}
		return massWeightedPos / mass;
	}

	glm::dvec3 velocity() {
		glm::float64 mass = bodies[primary]->mass;
		glm::dvec3 massWeightedVel = bodies[primary]->mass * bodies[primary]->velocity;
		for (size_t secondary : secondaries) {
			mass += bodies[secondary]->mass;
			massWeightedVel += bodies[secondary]->mass * bodies[secondary]->velocity;
		}
		return massWeightedVel / mass;
	}
};