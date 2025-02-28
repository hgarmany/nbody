#pragma once

#include "entity.h"
#include <deque>

enum gravType : uint8_t {
	POINT,
	OBLATE_SPHERE,
	RING
};

class GravityBody;

typedef struct orbit {
	GravityBody* parent;
	double semiMajorAxis;
	float eccentricity;
	float argPeriapsis;
	float anLongitude;
	float inclination;
	float meanAnomaly;

	std::function<float(float)> func = [this](float eccentricAnomaly) {
		return eccentricAnomaly - this->eccentricity * sinf(eccentricAnomaly) - this->meanAnomaly;
	};

	std::function<float(float)> derivFunc = [this](float eccentricAnomaly) {
		return 1.0f - this->eccentricity * cosf(eccentricAnomaly);
	};

	orbit(
		GravityBody* parent,
		double semiMajorAxis,
		float eccentricity,
		float argPeriapsis,
		float anLongitude,
		float inclination,
		float meanAnomaly
	) {
		this->parent = parent;
		this->semiMajorAxis = semiMajorAxis;
		this->eccentricity = eccentricity;
		this->argPeriapsis = argPeriapsis;
		this->anLongitude = anLongitude;
		this->inclination = inclination;
		this->meanAnomaly = meanAnomaly;
	}
} Orbit;

class Trail {
public:
	glm::mat4 rotation;
	glm::vec3 color;
	std::deque<glm::dvec3> queue;
	size_t parentIndex;

	Trail(glm::vec3 color = glm::vec3(0.0f), size_t parentIndex = -1)
		: color(color), parentIndex(parentIndex), rotation(glm::mat4(1.0f)) {}

	glm::dvec3 front() { return queue.front(); }
	glm::dvec3 back() { return queue.back(); }
	std::deque<glm::dvec3>::const_iterator begin() const { return queue.begin(); }
	std::deque<glm::dvec3>::const_iterator end() const { return queue.end(); }
	void push(const glm::dvec3 point) { queue.push_back(point); }
	void pop() { queue.pop_front(); }
	size_t size() { return queue.size(); }
};

class GravityBody : public Entity {
public:
	glm::float64 mass, radius, j2;
	float oblateness;
	Trail* trail;
	size_t parentIndex;
	gravType gravityType;

	GravityBody(glm::float64 mass = DBL_MIN);
	GravityBody(glm::float64 mass, Orbit orbit, size_t parentIndex);

	void initJ2();
	void draw(Shader& shader, uint8_t mode);
};