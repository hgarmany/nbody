#include "gravitybody.h"

GravityBody::GravityBody(glm::float64 mass) {
	parentIndex = -1;
	trail = nullptr;
	this->mass = mass;
	gravityType = POINT;
	radius = j2 = 0.0;
	oblateness = 0.0f;
	inertialTensor = angularMomentum = torque = glm::dvec3(0.0);
}

GravityBody::GravityBody(glm::float64 mass, Orbit orbit, size_t parentIndex) {
	this->parentIndex = parentIndex;
	trail = nullptr;
	this->mass = mass;
	gravityType = POINT;
	radius = j2 = 0.0;
	oblateness = 0.0f;
	inertialTensor = angularMomentum = torque = glm::dvec3(0.0);

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

	position = glm::dvec3((rotate * glm::dvec4(-positionInFrame.x, 0.0f, positionInFrame.y, 0.0f))) + orbit.parent->position;
	prevPosition = position;
	velocity = glm::dvec3((rotate * glm::dvec4(-velInFrame.x, 0.0f, velInFrame.y, 0.0f))) + orbit.parent->velocity;
}

glm::dvec3 GravityBody::getRotVelocity() {
	glm::dmat3 inverseInertialTensor(
		1.0 / inertialTensor.x, 0.0, 0.0,
		0.0, 1.0 / inertialTensor.y, 0.0,
		0.0, 0.0, 1.0 / inertialTensor.z
	);

	return inverseInertialTensor * (glm::transpose(glm::mat3_cast(rotQuat)) * angularMomentum);
}

void GravityBody::initJ2() {
	glm::dvec3 rotVelocity = getRotVelocity();
	j2 = (2 * oblateness - ((radius * radius * radius * rotVelocity.y * rotVelocity.y) / (G * mass))) / 3.0;
}

void GravityBody::initI() {
	if (gravityType == OBLATE_SPHERE) {
		glm::float64 polarRadius = radius * (1 - oblateness);
		glm::float64 equatorialMoment = 0.2 * mass * (radius * radius + polarRadius * polarRadius);
		glm::float64 polarMoment = 0.4 * mass * radius * radius;
		inertialTensor = glm::dvec3(equatorialMoment, polarMoment, equatorialMoment);
	}
	else if (gravityType == CUBOID) {
		glm::float64 xSq = scale.x * scale.x;
		glm::float64 ySq = scale.y * scale.y;
		glm::float64 zSq = scale.z * scale.z;
		inertialTensor = (scale.x * scale.y * scale.z / 12) * glm::dvec3(ySq + zSq, xSq + zSq, xSq + ySq);
		//inertialTensor = (mass / 12) * glm::dvec3(ySq + zSq, xSq + zSq, xSq + ySq);
	}
}

void GravityBody::draw(Shader& shader, uint8_t mode) {
	glUniform1f(shader.uniforms[OBJ_OBLATE], oblateness);

	((Entity*)this)->draw(shader, mode);
}