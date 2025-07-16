#pragma once

#include "gravitybody.h"

class Barycenter {
protected:
	size_t primary;
public:
	virtual glm::float64 mass() = 0;
	virtual glm::dvec3 position() = 0;
	virtual glm::dvec3 velocity() = 0;
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
		else if (bodies[a]->mass > bodies[b]->mass) {
			primary = a;
			secondary = b;

			bodies[a]->barycenter = this;
		}
		else {
			primary = b;
			secondary = a;

			bodies[b]->barycenter = this;
		}
	}

	glm::float64 mass() {
		return bodies[primary]->mass + bodies[secondary]->mass;
	}

	glm::dvec3 position() {
		return (bodies[primary]->mass * bodies[primary]->position + 
			bodies[secondary]->mass * bodies[secondary]->position) / 
			(bodies[primary]->mass + bodies[secondary]->mass);
	}

	glm::dvec3 velocity() {
		return (bodies[primary]->mass * bodies[primary]->velocity +
			bodies[secondary]->mass * bodies[secondary]->velocity) /
			(bodies[primary]->mass + bodies[secondary]->mass);
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