#include "physics.h"
#include "barycenter.h"
#include "logger.h"

std::vector<std::unique_ptr<Logger>> loggers;

uint8_t targetRotation = 0;

Camera camera;
Camera pipCam;

std::atomic<bool> running(true);
std::condition_variable physicsDone, physicsStart;
std::mutex physicsMutex;

bool hasPhysics = false;
bool doTrails = true;

double frameTime = 0.0;
double elapsedTime = 0.0;
double timeStep = 1e5;
size_t maxTrailLength = 2500;

std::unordered_map<size_t, double> lastTheta;

static void mergeNearBodies() {
	for (int i = 0; i < bodies.size(); i++) {
		for (int j = i + 1; j < bodies.size(); j++) {
			// bodies within a medium distance are merged into one body
			double minDist = fmax(bodies[i]->radius, bodies[j]->radius) / 5.0;
			if (glm::distance(bodies[i]->position, bodies[j]->position) < minDist) {
				std::shared_ptr<GravityBody> a = bodies[i];
				std::shared_ptr<GravityBody> b = bodies[j];

				// mass relative to the 2-body system
				double m1 = a->mass / (a->mass + b->mass);
				double m2 = b->mass / (a->mass + b->mass);
				
				// set mass-centered ballistic properties
				glm::dvec3 center = a->position * m1 + b->position * m2;
				glm::dvec3 vel = a->velocity * m1 + b->velocity * m2;
				a->position = center;
				a->velocity = vel;
				a->mass += b->mass;

				// resize volume
				double r1 = a->radius;
				double r2 = b->radius;
				a->radius = cbrt(r1 * r1 * r1 + r2 * r2 * r2);
				a->scale = glm::dvec3(a->radius);
				bodies.erase(bodies.begin() + j);
				j--;
			}
		}
	}
}

static bool absoluteOrbitalAngle(size_t index) {
	bool result = false;
	glm::dvec3 r0, r1, velParent, velOrbiter;
	double parentMass;

	if (bodies[bodies[index]->parentIndex]->barycenter) {
		// replace parent object parameters with those of its barycenter
		// i.e. a planet with a massive moon where we wish to see the moon's orbit relative to the COM
		Barycenter* bary = bodies[bodies[index]->parentIndex]->barycenter;
		r0 = bary->position(bodies);
		velParent = bary->velocity(bodies);
		parentMass = bary->apparentMass(bodies, index);
	}
	else {
		r0 = bodies[bodies[index]->parentIndex]->position;
		velParent = bodies[bodies[index]->parentIndex]->velocity;
		parentMass = bodies[bodies[index]->parentIndex]->mass;
	}

	if (bodies[index]->barycenter) {
		// replace orbiter object parameters with those of its barycenter
		// i.e. a planet with a massive moon that we wish to track as a single object orbiting a star
		Barycenter* bary = bodies[index]->barycenter;
		r1 = bary->position(bodies);
		velOrbiter = bary->velocity(bodies);
	}
	else {
		r1 = bodies[index]->position;
		velOrbiter = bodies[index]->velocity;
	}

	glm::dvec3 thDir = glm::normalize(r1 - r0); // direction of position relative to orbital focus
	glm::dvec3 N = glm::normalize(glm::cross(thDir, velOrbiter - velParent));
	glm::dvec3 thZeroDir = glm::normalize(glm::dvec3(1, 0, 0) - N * N.x);
	double theta = atan2(glm::dot(N, glm::cross(thZeroDir, thDir)), glm::dot(thZeroDir, thDir));
	
	if (theta >= 0 && lastTheta[index] < 0)
		result = true;
	
	lastTheta[index] = theta;

	return result;
}

glm::dmat4 relativeRotationalMatrix(
	context& list, 
	const std::shared_ptr<GravityBody>& subject, 
	const std::shared_ptr<GravityBody>& reference, 
	bool detranslate)
{
	// develops a rotational matrix to transform subject coordinates to a reference frame 
	// in which the reference body and its parent both lie along the x axis
	glm::dmat4 rotate(1.0);
	if (reference != list[subject->parentIndex]) {
		size_t aParentIndex = reference->parentIndex;
		glm::dvec3 a = glm::normalize(reference->position - list[aParentIndex]->position);
		glm::dvec3 n = glm::cross(a, reference->velocity);

		glm::dvec3 b(1.0, 0.0, 0.0);

		double angle = acos(glm::dot(a, b));
		if (glm::dot(n, glm::cross(a, b)) < 0)
			angle *= -1;

		// shaders will detranslate spatial adjustments
		if (detranslate)
			rotate = glm::rotate(rotate, -angle, n);
		else
			rotate = glm::rotate(rotate, angle, n);
	}
	return rotate;
}

glm::dvec3 orbitalVelocity(size_t parent, size_t orbiter) {
	glm::dvec3 gravitation = bodies[parent]->position - bodies[orbiter]->position;
	double distance = glm::length(gravitation);
	double magnitude = sqrt(G * bodies[parent]->mass / distance);
	return magnitude * glm::normalize(glm::cross(glm::dvec3(0, 1, 0), gravitation));
}

static glm::vec3 eccentricityVector(size_t parent, size_t orbiter) {
	glm::dvec3 position = bodies[orbiter]->position - bodies[parent]->position;
	glm::dvec3 velocity = bodies[orbiter]->velocity - bodies[parent]->velocity;

	glm::dvec3 orbitalMomentum = glm::cross(position, velocity);
	return glm::cross(velocity, orbitalMomentum) / (G * bodies[parent]->mass) - position / glm::length(position);
}

static double semiMajorAxis(size_t parent, size_t orbiter) {
	double distance = glm::distance(bodies[orbiter]->position, bodies[parent]->position);
	double speed = glm::distance(bodies[orbiter]->velocity, bodies[parent]->velocity);
	return 1.0 / (2.0 / distance - speed * speed / (G * bodies[parent]->mass));
}

static void gravitationalForce(std::shared_ptr<GravityBody>& a, std::shared_ptr<GravityBody>& b) {
	glm::dvec3 displacement = b->position - a->position;
	double distance = glm::length(displacement);
	glm::dvec3 direction = glm::normalize(displacement);
	glm::dvec3 fieldLine = (G * direction) / (distance * distance);
	glm::dvec3 accelerationA = b->mass * fieldLine;

	if (b->gravityType == OBLATE_SPHERE) {
		// oblate perturbations on a by b, using MacCullagh's formula
		glm::dvec3 axisOfRotation = glm::normalize(b->rotQuat * glm::dvec3(0.0, 1.0, 0.0));
		double cosTheta = glm::dot(-direction, axisOfRotation);
		double sin2Theta = 1 - cosTheta * cosTheta;
		double radiusOverDistance = b->radius / distance;
		accelerationA *= 1 - 3.0 * b->j2 * radiusOverDistance * radiusOverDistance * (3.0 * sin2Theta - 1.0);

		// torque
		b->nextTorque += 3.0 * G * a->mass * b->mass * b->j2 * radiusOverDistance * radiusOverDistance / distance
			* cosTheta * glm::cross(-direction, axisOfRotation);
	}

	glm::dvec3 accelerationB = -a->mass * fieldLine;

	if (a->gravityType == OBLATE_SPHERE) {
		// oblate perturbations on b by a, using MacCullagh's formula
		glm::dvec3 axisOfRotation = glm::normalize(a->rotQuat * glm::dvec3(0.0, 1.0, 0.0));
		double cosTheta = glm::dot(direction, axisOfRotation);
		double sin2Theta = 1 - cosTheta * cosTheta;
		double radiusOverDistance = a->radius / distance;
		accelerationB *= 1 - 3.0 * a->j2 * radiusOverDistance * radiusOverDistance * (3.0 * sin2Theta - 1.0);

		// torque
		a->nextTorque += 3.0 * G * a->mass * b->mass * a->j2 * radiusOverDistance * radiusOverDistance / distance
			* cosTheta * glm::cross(direction, axisOfRotation);
	}

	a->acceleration += accelerationA;
	b->acceleration += accelerationB;
}

static void updateBodies(context& bodies, double deltaTime) {
	double fullDt = timeStep * deltaTime;
	double halfDt = fullDt * 0.5;

	// Update velocities and positions by half-step, clear accelerations
	#pragma omp parallel for
	for (std::shared_ptr<GravityBody>& body : bodies) {
		body->velocity += body->acceleration * halfDt;

		body->prevPosition = body->position;
		body->position += body->velocity * fullDt;

		body->torque = body->nextTorque;
		body->angularMomentum += body->torque * halfDt;

		body->rotateRK4(fullDt);
		
		body->acceleration = body->nextTorque = glm::dvec3(0.0);
	}

	// Compute forces between particles
	for (size_t i = 0; i < bodies.size(); ++i) {
		for (size_t j = i + 1; j < bodies.size(); ++j)
			gravitationalForce(bodies[i], bodies[j]);
	}

	// Update velocities to full-step using the new accelerations
	#pragma omp parallel for
	for (std::shared_ptr<GravityBody>& body : bodies) {
		body->velocity += body->acceleration * halfDt;
		body->angularMomentum += body->torque * halfDt;
	}

	elapsedTime += fullDt;
}

static void tracePath(size_t parent, size_t orbiter) {
	bool doLoop = true;

	glm::dvec3 parentPos;
	Barycenter* bary = frameBodies[parent]->barycenter;
	if (bary)
		parentPos = bary->position(frameBodies);
	else
		parentPos = frameBodies[parent]->position;
	glm::dvec3 orbiterPos = frameBodies[orbiter]->position;
	Trail* orbiterTrail = frameBodies[orbiter]->trail;

	// remove any trail points behind the body's position
	while (doLoop && orbiterTrail->size() > 1 &&
		parent == frameBodies[orbiter]->parentIndex) {

		glm::dvec3 a = glm::normalize(orbiterTrail->front());
		glm::dvec3 b = glm::normalize(orbiterTrail->back());

		glm::dvec3 n = glm::cross(b, orbiterPos - parentPos);

		// get angle between the last point added to the trail and the start of the trail
		double angle = acos(glm::dot(a, b));
		if (glm::dot(n, glm::cross(a, b)) < 0)
			angle *= -1;

		// get angle between the body's current position and the start of the trail
		b = glm::normalize(orbiterPos - parentPos);
		double angle2 = acos(glm::dot(a, b));
		if (glm::dot(n, glm::cross(a, b)) < 0)
			angle2 *= -1;

		if (angle < 0 && angle2 >= 0 || std::isnan(angle) || std::isnan(angle2))
			orbiterTrail->pop();
		else
			doLoop = false;
	}

	glm::dvec3 relativePosition =
		glm::dmat3(relativeRotationalMatrix(bodies, bodies[orbiter], bodies[parent])) *
		glm::dvec3(orbiterPos - parentPos);

	if (orbiterTrail->size() == 0 || glm::distance(orbiterTrail->back(), relativePosition) * 2 > frameBodies[orbiter]->radius)
		orbiterTrail->push(relativePosition);
}

// draw an orbit around a given mass where the orbiter has a given position and velocity relative to it
static void drawEllipse(Trail* trail, const glm::dvec3& position, const glm::dvec3& velocity, double parentMass) {
	double distance = glm::length(position);
	double speed = glm::length(velocity);

	double gravParam = G * parentMass;

	glm::dvec3 orbitalMomentum = glm::cross(position, velocity);
	glm::dvec3 h_hat = glm::normalize(orbitalMomentum);
	glm::dvec3 eccVector = glm::cross(velocity, orbitalMomentum) / gravParam - position / distance;

	double eccentricity = glm::length(eccVector);
	glm::dvec3 e_hat = glm::normalize(eccVector);

	double semiMajorAxis = 1.0 / (2.0 / distance - speed * speed / gravParam);

	trail->queue.clear();
	double perimeter = ellipsePerimeter(semiMajorAxis, (float)eccentricity);
	double thetaStep = 0.002 * pi;
	double thetaMin = -pi;
	double thetaMax = pi;

	// hyperbolic trajectory
	if (eccentricity >= 1.0) {
		thetaMax = acos(-1.0 / eccentricity) * 0.99;
		thetaMin = -thetaMax * 0.99;
	}

	glm::dvec3 thDir = glm::normalize(position); // direction of position relative to orbital focus
	glm::dvec3 N = glm::normalize(glm::cross(thDir, velocity));
	double bodyTheta = atan2(glm::dot(N, glm::cross(e_hat, thDir)), glm::dot(e_hat, thDir));

	// a new trail is drawn by stepping through the path of the conic for thetaMin <= th= < thetaMax
	for (double theta = thetaMin; theta < thetaMax; theta += thetaStep) {
		// polar radius
		double r = semiMajorAxis * (1 - eccentricity * eccentricity) / (1 + eccentricity * cos(theta));
		glm::dvec3 trailPoint = e_hat * (r * cos(theta)) + glm::cross(h_hat, e_hat) * (r * sin(theta));
		trail->push(trailPoint);

		// fix orbital path to location of orbiter
		if (bodyTheta > theta && bodyTheta < theta + thetaStep) {
			r = semiMajorAxis * (1 - eccentricity * eccentricity) / (1 + eccentricity * cos(bodyTheta));
			trailPoint = e_hat * (r * cos(bodyTheta)) + glm::cross(h_hat, e_hat) * (r * sin(bodyTheta));
			trail->push(trailPoint);
		}
	}

	if (eccentricity < 1.0) {
		trail->push(trail->front());
	}
	else {
		double r = semiMajorAxis * (1 - eccentricity * eccentricity) / (1 + eccentricity * cos(thetaMax));
		glm::dvec3 trailPoint = e_hat * (r * cos(thetaMax)) + glm::cross(h_hat, e_hat) * (r * sin(thetaMax));
		trail->push(trailPoint);
	}
}

static void ellipticalPath(context& bodies, Barycenter* parent, size_t orbiter) {
	// for drawing the path of a barycenter's primary around that barycenter
	glm::dvec3 position = bodies[orbiter]->position - parent->position(frameBodies);
	glm::dvec3 velocity = bodies[orbiter]->velocity - parent->velocity(frameBodies);

	drawEllipse(parent->primaryOrbit, position, velocity, parent->apparentMass(frameBodies, orbiter));
}

static void ellipticalPath(context& bodies, size_t parent, size_t orbiter) {
	glm::dvec3 r0, r1, velParent, velOrbiter;
	double parentMass;

	if (bodies[parent]->barycenter) {
		// replace parent object parameters with those of its barycenter
		// i.e. a planet with a massive moon where we wish to see the moon's orbit relative to the COM
		Barycenter* bary = bodies[parent]->barycenter;
		r0 = bary->position(frameBodies);
		velParent = bary->velocity(frameBodies);
		parentMass = bary->apparentMass(frameBodies, orbiter);
	}
	else {
		r0 = bodies[parent]->position;
		velParent = bodies[parent]->velocity;
		parentMass = bodies[parent]->mass;
	}

	if (bodies[orbiter]->barycenter) {
		// replace orbiter object parameters with those of its barycenter
		// i.e. a planet with a massive moon that we wish to track as a single object orbiting a star
		Barycenter* bary = bodies[orbiter]->barycenter;
		r1 = bary->position(frameBodies);
		velOrbiter = bary->velocity(frameBodies);
	}
	else {
		r1 = bodies[orbiter]->position;
		velOrbiter = bodies[orbiter]->velocity;
	}

	glm::dvec3 position = r1 - r0;
	glm::dvec3 velocity = velOrbiter - velParent;

	drawEllipse(bodies[orbiter]->trail, position, velocity, parentMass);
}

void updateTrails(context& bodies) {
	#pragma omp parallel for
	for (size_t i = 0; i < bodies.size(); i++) {
		std::shared_ptr<GravityBody> body = bodies[i];
		Trail* trail = body->trail;

		if (bodies[i]->barycenter)
			ellipticalPath(bodies, bodies[i]->barycenter, i);

		if (trail) {
			size_t parentIndex = body->trail->parentIndex;

			// trail relative to parent body
			if (parentIndex != -1) {
				if (i == bodies.size() - 1)
					tracePath(parentIndex, i);
				else
					ellipticalPath(bodies, parentIndex, i);
			}
			// trail relative to world space
			else {
				trail->push(body->position);
				while (trail->size() > maxTrailLength)
					trail->pop();
			}
		}
	}
}

void initLoggers() {
	std::unique_ptr<Logger> logEarth = std::make_unique<Logger>(
		"test.csv", 12, SEMI_MAJOR_AXIS | ECCENTRICITY | TORQUE);
	logEarth->addCondition(absoluteOrbitalAngle);
	loggers.push_back(std::move(logEarth));
}

void physicsLoop() {
	double totalTimeElapsed = 0.0;

	Clock::time_point lastLoopTime = Clock::now();

	while (running) {
		Clock::time_point currentTime = Clock::now();
		double deltaTime =
			std::chrono::duration<double>(currentTime - lastLoopTime).count();
		lastLoopTime = currentTime;

		frameTime = deltaTime * timeStep;
		if (frameTime < MAX_PHYSICS_TIME) {
			if (hasPhysics) {
				totalTimeElapsed += frameTime;
				
				updateBodies(bodies, deltaTime);

				// write astronomical data to file
				for (std::unique_ptr<Logger>& logger : loggers)
					logger->logIfNeeded(totalTimeElapsed / (31.7791f * 3600.0f));

				// data is ready for renderer to access
				physicsDone.notify_one();
			}
			else {
				// thread waits while physics are disabled
				std::unique_lock<std::mutex> lock(physicsMutex);
				physicsStart.wait(lock);
			}
		}
		else
			printf("discarded physics frame: %.2f ms\n", deltaTime * 1000);
	}
}