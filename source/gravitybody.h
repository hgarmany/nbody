#pragma once

#include "entity.h"
#include <deque>

enum gravType : uint8_t {
	POINT,
	CUBOID,
	OBLATE_SPHERE,
	RING
};

class GravityBody;
class Barycenter;

extern std::vector<std::shared_ptr<GravityBody>> bodies, frameBodies;

typedef struct orbit {
	std::shared_ptr<GravityBody> parent;
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
		std::shared_ptr<GravityBody> parent,
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

struct MaterialLayer {
	double shearMod;	// shear modulus in Pa
	double viscosity;	// viscosity in Pa·s
	double innerRadius, outerRadius;	// layer bounds as fractions of body radius
};

class GravityBody : public Entity {
public:
	glm::dvec3 momentOfInertia, angularMomentum, torque;
	Trail* trail;
	size_t parentIndex;
	glm::float64 mass, radius, j2;
	float oblateness, loveNumber, qualityFactor;
	gravType gravityType;
	Barycenter* barycenter;

	GravityBody(glm::float64 mass = DBL_MIN);
	GravityBody(glm::float64 mass, Orbit orbit, size_t parentIndex);

	//std::vector<MaterialLayer> makeLayers();
	//void initTidalParams();
	void initJ2();
	void initI();

	glm::dvec3 getRotVelocity();
	void rotateRK4(double dt);
	
	void draw(Shader& shader, uint8_t mode);
private:
	glm::dquat rotateDeriv(const glm::dquat& orientation, const glm::dvec3& momentum);
};