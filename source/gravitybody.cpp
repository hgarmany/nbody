#include "gravitybody.h"

GravityBody::GravityBody(glm::float64 mass) {
	parentIndex = -1;
	trail = nullptr;
	this->mass = mass;
	gravityType = POINT;
	radius = j2 = 0.0;
	oblateness = 0.0f;
	momentOfInertia = angularMomentum = torque = glm::dvec3(0.0);
}

GravityBody::GravityBody(glm::float64 mass, Orbit orbit, size_t parentIndex) {
	this->parentIndex = parentIndex;
	trail = nullptr;
	this->mass = mass;
	gravityType = POINT;
	radius = j2 = 0.0;
	oblateness = 0.0f;
	momentOfInertia = angularMomentum = torque = glm::dvec3(0.0);

	// equations sourced from René Schwarz "Keplerian Orbit Elements -> Cartesian State Vectors"
	float eccentricAnomaly = rootSolver<float>(orbit.func, orbit.derivFunc, 0.0f);
	float trueAnomaly = 2 * atan2f(
		sqrtf(1.0f + orbit.eccentricity) * sinf(eccentricAnomaly * 0.5f),
		sqrtf(1.0f - orbit.eccentricity) * cosf(eccentricAnomaly * 0.5f));
	double gravParameter = G * (mass + orbit.parent->mass);
	double distance = orbit.semiMajorAxis * (1.0 - (double)(orbit.eccentricity * cosf(eccentricAnomaly)));

	// motion relative to the orbital frame
	glm::dvec2 velInFrame = sqrt(gravParameter * orbit.semiMajorAxis) / distance *
		glm::dvec2(-sinf(eccentricAnomaly), sqrtf(1 - orbit.eccentricity * orbit.eccentricity) * cosf(eccentricAnomaly));
	glm::dvec2 positionInFrame = distance * glm::dvec2(cosf(trueAnomaly), sinf(trueAnomaly));

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

	glm::dvec3 shared_dV = glm::dvec3((rotate * glm::dvec4(-velInFrame.x, 0.0f, velInFrame.y, 0.0f))) * orbit.parent->mass / (mass + orbit.parent->mass);

	velocity = orbit.parent->velocity + shared_dV;
	orbit.parent->velocity -= shared_dV * mass / orbit.parent->mass;

	//velocity = glm::dvec3((rotate * glm::dvec4(-velInFrame.x, 0.0f, velInFrame.y, 0.0f))) + orbit.parent->velocity;
	position = glm::dvec3((rotate * glm::dvec4(-positionInFrame.x, 0.0f, positionInFrame.y, 0.0f))) + orbit.parent->position;
	prevPosition = position;
}

// get rotation velocities in body space
glm::dvec3 GravityBody::getRotVelocity() {
	glm::dmat3 inverseInertialTensor(
		1.0 / momentOfInertia.x, 0.0, 0.0,
		0.0, 1.0 / momentOfInertia.y, 0.0,
		0.0, 0.0, 1.0 / momentOfInertia.z
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
		momentOfInertia = glm::dvec3(equatorialMoment, polarMoment, equatorialMoment);
	}
	else if (gravityType == CUBOID) {
		glm::float64 xSq = scale.x * scale.x;
		glm::float64 ySq = scale.y * scale.y;
		glm::float64 zSq = scale.z * scale.z;
		momentOfInertia = (mass / 12) * glm::dvec3(ySq + zSq, xSq + zSq, xSq + ySq);
	}
	else {
		momentOfInertia = glm::dvec3(0.4 * mass * radius * radius);
	}
}

glm::dquat GravityBody::rotateDeriv(const glm::dquat& orientation, const glm::dvec3& momentum) {
	glm::dmat3 rotMatrix = glm::mat3_cast(orientation);

	glm::dvec3 momentumWorld = glm::transpose(rotMatrix) * momentum; // angular momentum in world frame
	glm::dvec3 velocityBody = {
		momentumWorld.x / momentOfInertia.x,
		momentumWorld.y / momentOfInertia.y,
		momentumWorld.z / momentOfInertia.z
	}; // angular velocity in body frame
	glm::dvec3 velocityWorld = rotMatrix * velocityBody; // angular velocity in world frame

	return glm::dquat(0, velocityWorld.x, velocityWorld.y, velocityWorld.z) * orientation * 0.5;
}

void GravityBody::rotateRK4(double dt) {
	glm::dquat k1 = rotateDeriv(rotQuat, angularMomentum);
	glm::dquat k2 = rotateDeriv(glm::normalize(rotQuat + k1 * dt * 0.5), angularMomentum + torque * dt * 0.5);
	glm::dquat k3 = rotateDeriv(glm::normalize(rotQuat + k2 * dt * 0.5), angularMomentum + torque * dt * 0.5);
	glm::dquat k4 = rotateDeriv(glm::normalize(rotQuat + k3 * dt), angularMomentum + torque * dt);

	glm::dquat rotationChange = (k1 + k2 * 2.0 + k3 * 2.0 + k4) * (dt / 6.0);
	rotQuat = glm::normalize(rotQuat + rotationChange);
	angularMomentum += torque * dt;
}

void GravityBody::draw(Shader& shader, uint8_t mode) {
	glUniform1f(shader.uniforms[OBJ_OBLATE], oblateness);

	((Entity*)this)->draw(shader, mode);
}