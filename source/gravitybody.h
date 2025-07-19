#pragma once

#include "entity.h"
#include "trail.h"

enum gravType : uint8_t {
	POINT,
	CUBOID,
	OBLATE_SPHERE,
	RING
};

class Barycenter {
protected:
	size_t primary;
public:
	Trail* primaryOrbit;
	virtual void add(size_t secondary) = 0;
	virtual glm::float64 mass() = 0;
	virtual glm::dvec3 position() = 0;
	virtual glm::dvec3 velocity() = 0;
	virtual glm::float64 apparentMass(size_t observer) = 0;
};

class GravityBody;

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

extern std::vector<std::shared_ptr<GravityBody>> bodies, frameBodies;

struct MaterialLayer {
	double shearMod;	// shear modulus in Pa
	double viscosity;	// viscosity in Pa·s
	double innerRadius, outerRadius;	// layer bounds as fractions of body radius
};