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

class GravityBody : public Entity {
public:
	glm::vec3 trailColor;
	std::deque<glm::dvec3>* trail;
	glm::float64 mass;
	glm::float64 radius;
	size_t parentIndex;

	GravityBody(glm::float64 mass) {
		parentIndex = -1;
		trail = nullptr;
		trailColor = glm::vec3(0.0f);
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
		trailColor = glm::vec3(0.0f);
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
		/*
		glm::dmat2x3 rotation(
			cosZ * cosY - sinZ * cosX * sinY,
			cosZ * sinY + sinZ * cosX * cosY,
			sinZ * sinX,

			-(sinZ * cosY + cosZ * cosX * sinY),
			cosZ * cosX * sinY - sinZ * sinY,
			cosZ * sinX
		);
		*/
		glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), orbit.argPeriapsis, glm::vec3(0.0f, 1.0f, 0.0f));
		rotate = glm::rotate(rotate, orbit.inclination, glm::vec3(1.0f, 0.0f, 0.0f));
		rotate = glm::rotate(rotate, orbit.anLongitude, glm::vec3(0.0f, 1.0f, 0.0f));

		position = glm::dvec3((rotate * glm::dvec4(-positionInFrame.x, 0.0f, positionInFrame.y, 0.0f))) + orbit.parent->position;
		prevPosition = position;
		velocity = glm::dvec3((rotate * glm::dvec4(-velInFrame.x, 0.0f, velInFrame.y, 0.0f))) + orbit.parent->velocity;

	}
};