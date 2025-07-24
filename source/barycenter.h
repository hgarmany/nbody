#pragma once

#include "gravitybody.h"

class TwoBodyBarycenter : public Barycenter {
protected:
	size_t secondary;
public:
	TwoBodyBarycenter(size_t a, size_t b);

	void add(size_t secondary) {
		this->secondary = secondary;
	}

	glm::float64 mass();
	glm::dvec3 position();
	glm::dvec3 velocity();
	glm::float64 apparentMass(size_t observer);
	void positionOffset(glm::dvec3 offset);
	void velocityOffset(glm::dvec3 offset);
};

class ComplexBarycenter : public Barycenter {
protected:
	std::vector<size_t> secondaries;
public:
	ComplexBarycenter(size_t primary, size_t secondary);
	ComplexBarycenter(size_t primary, std::vector<size_t> secondaries);
	ComplexBarycenter(std::vector<size_t> indices);

	void add(size_t secondary) {
		if (std::find(secondaries.begin(), secondaries.end(), secondary) == secondaries.end())
			secondaries.push_back(secondary);
	}

	glm::float64 mass();
	glm::dvec3 position();
	glm::dvec3 velocity();
	glm::float64 apparentMass(size_t observer);
	void positionOffset(glm::dvec3 offset);
	void velocityOffset(glm::dvec3 offset);
};