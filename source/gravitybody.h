#pragma once

#include "entity.h"
#include "trail.h"

enum gravType : uint8_t {
	POINT,
	CUBOID,
	OBLATE_SPHERE,
	RING
};

class GravityBody;
using context = std::vector<std::shared_ptr<GravityBody>>;

class Barycenter {
protected:
	size_t primary;
public:
	Trail* primaryOrbit;
	virtual void add(size_t secondary) = 0;
	virtual double mass(context& context) = 0;
	virtual glm::dvec3 position(context& context) = 0;
	virtual glm::dvec3 velocity(context& context) = 0;
	virtual double apparentMass(context& context, size_t observer) = 0;
	virtual void positionOffset(context& context, glm::dvec3 offset) = 0;
	virtual void velocityOffset(context& context, glm::dvec3 offset) = 0;
};

typedef struct orbit {
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
		double semiMajorAxis,
		float eccentricity,
		float argPeriapsis,
		float anLongitude,
		float inclination,
		float meanAnomaly
	) {
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
	glm::dvec3 momentOfInertia, angularMomentum, torque, nextTorque;
	Trail* trail;
	size_t parentIndex;
	double mass, radius, j2;
	float oblateness;//, loveNumber, qualityFactor;
	gravType gravityType;
	Barycenter* barycenter;

	GravityBody(double mass = DBL_MIN);
	GravityBody(double mass, Orbit orbit, size_t parentIndex, bool addToBary = true);

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

extern context bodies, frameBodies;

struct MaterialLayer {
	double shearMod;	// shear modulus in Pa
	double viscosity;	// viscosity in Pa·s
	double innerRadius, outerRadius;	// layer bounds as fractions of body radius
};