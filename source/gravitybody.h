#pragma once

#include "entity.h"
#include <deque>

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
	glm::float64 mass;
	glm::float64 radius;
	Trail* trail;
	size_t parentIndex;

	GravityBody(glm::float64 mass = DBL_MIN) {
		parentIndex = -1;
		trail = nullptr;
		this->mass = mass;
		radius = 0.0;
		position = glm::dvec3(0.0);
		velocity = glm::dvec3(0.0);
		acceleration = glm::dvec3(0.0);
		
		modelMatrix = glm::dmat4(0.0);
	}

	GravityBody(glm::float64 mass, Orbit orbit, size_t parentIndex) {
		this->parentIndex = parentIndex;
		trail = nullptr;
		this->mass = mass;
		radius = 0.0;
		acceleration = glm::dvec3(0.0);
		modelMatrix = glm::dmat4(0.0);

		// equations sourced from René Schwarz "Keplerian Orbit Elements -> Cartesian State Vectors"
		float eccentricAnomaly = rootSolver<float>(orbit.func, orbit.derivFunc, 0.0f);
		float trueAnomaly = 2 * atan2f(
			sqrtf(1.0f + orbit.eccentricity) * sinf(eccentricAnomaly * 0.5f),
			sqrtf(1.0f - orbit.eccentricity) * cosf(eccentricAnomaly * 0.5f));
		double gravParameter = G * (mass + orbit.parent->mass);
		double distance = orbit.semiMajorAxis * (1.0 - (double)(orbit.eccentricity * cosf(eccentricAnomaly)));
		
		// motion relative to the orbital frame
		glm::dvec2 positionInFrame = distance * glm::dvec2(cosf(trueAnomaly), sinf(trueAnomaly));
		glm::dvec2 velInFrame = sqrt(gravParameter * orbit.semiMajorAxis) / distance *
			glm::dvec2(-sinf(eccentricAnomaly), sqrtf(1 - orbit.eccentricity * orbit.eccentricity) * cosf(eccentricAnomaly));
	
		// motion rotated out to world space
		float sinX = sinf(orbit.inclination);
		float sinY = sinf(orbit.anLongitude);
		float sinZ = sinf(orbit.argPeriapsis);
		float cosX = cosf(orbit.inclination);
		float cosY = cosf(orbit.anLongitude);
		float cosZ = cosf(orbit.argPeriapsis);
		
		glm::mat4 rotate = glm::mat4(
			cosY * cosZ - sinY * cosX * sinZ,
			sinY * sinX,
			-cosY * sinZ - sinY * cosX * cosZ,
			0.0f,

			sinX * sinZ,
			cosX,
			sinX * cosZ,
			0.0f,

			sinY * cosZ + cosY * cosX * sinZ,
			-cosY * sinX,
			-sinY * sinZ + cosY * cosX * cosZ,
			0.0f,

			0.0f, 0.0f, 0.0f, 1.0f
		);

		/*
		glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), orbit.argPeriapsis, glm::vec3(0.0f, 1.0f, 0.0f));
		rotate = glm::rotate(rotate, orbit.inclination, glm::vec3(1.0f, 0.0f, 0.0f));
		rotate = glm::rotate(rotate, orbit.anLongitude, glm::vec3(0.0f, 1.0f, 0.0f));
		*/

		position = glm::dvec3((rotate * glm::dvec4(-positionInFrame.x, 0.0f, positionInFrame.y, 0.0f))) + orbit.parent->position;
		prevPosition = position;
		velocity = glm::dvec3((rotate * glm::dvec4(-velInFrame.x, 0.0f, velInFrame.y, 0.0f))) + orbit.parent->velocity;

	}
};