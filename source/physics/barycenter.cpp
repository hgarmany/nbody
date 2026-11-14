#include "barycenter.h"

TwoBodyBarycenter::TwoBodyBarycenter(size_t a, size_t b) {
	if (bodies.size() < a || bodies.size() < b) {
		primary = -1;
		secondary = -1;
	}
	else if (bodies[a]->mass >= bodies[b]->mass) {
		primary = a;
		secondary = b;

		bodies[a]->barycenter = this;
	}
	else {
		primary = b;
		secondary = a;

		bodies[b]->barycenter = this;
	}

	glm::vec3 newColor = bodies[primary]->trail->color;

	if (primary % 3 == 0)
		newColor.r = newColor.r > 0.5f ? newColor.r - 0.5f : newColor.r + 0.5f;
	if (primary % 3 == 1)
		newColor.g = newColor.g > 0.5f ? newColor.g - 0.5f : newColor.g + 0.5f;
	if (primary % 3 == 2)
		newColor.b = newColor.b > 0.5f ? newColor.b - 0.5f : newColor.b + 0.5f;

	primaryOrbit = new Trail(newColor, primary);
}

double TwoBodyBarycenter::mass(context& context) {
	return context[primary]->mass + context[secondary]->mass;
}

glm::dvec3 TwoBodyBarycenter::position(context& context) {
	return (context[primary]->mass * context[primary]->position +
		context[secondary]->mass * context[secondary]->position) /
		(context[primary]->mass + context[secondary]->mass);
}

glm::dvec3 TwoBodyBarycenter::velocity(context& context) {
	return (context[primary]->mass * context[primary]->velocity +
		context[secondary]->mass * context[secondary]->velocity) /
		(context[primary]->mass + context[secondary]->mass);
}

double TwoBodyBarycenter::apparentMass(context& context, size_t observer) {
	glm::dvec3 com = (context[primary]->mass * context[primary]->position +
		context[secondary]->mass * context[secondary]->position) /
		(context[primary]->mass + context[secondary]->mass);
	double d1 = glm::distance(context[primary]->position, com);
	double d2 = glm::distance(context[secondary]->position, com);

	if (observer == primary)
		return context[secondary]->mass * (d1 * d1) / ((d1 + d2) * (d1 + d2));
	if (observer == secondary)
		return context[primary]->mass * (d2 * d2) / ((d1 + d2) * (d1 + d2));
	return context[primary]->mass + context[secondary]->mass;
}

void TwoBodyBarycenter::positionOffset(context& context, glm::dvec3 offset) {
	context[primary]->position -= offset;
	context[secondary]->position -= offset;
}

void TwoBodyBarycenter::velocityOffset(context& context, glm::dvec3 offset) {
	context[primary]->velocity -= offset;
	context[secondary]->velocity -= offset;
}

ComplexBarycenter::ComplexBarycenter(size_t primary, size_t secondary) {
	this->primary = primary;
	this->secondaries.push_back(secondary);

	bodies[primary]->barycenter = this;

	Trail* primaryTrail = bodies[primary]->trail;
	if (primaryTrail) {
		glm::vec3 newColor = bodies[primary]->trail->color;

		if (primary % 3 == 0)
			newColor.r = newColor.r > 0.5f ? newColor.r - 0.5f : newColor.r + 0.5f;
		if (primary % 3 == 1)
			newColor.g = newColor.g > 0.5f ? newColor.g - 0.5f : newColor.g + 0.5f;
		if (primary % 3 == 2)
			newColor.b = newColor.b > 0.5f ? newColor.b - 0.5f : newColor.b + 0.5f;

		primaryOrbit = new Trail(newColor, primary);
	}
	else {
		primaryOrbit = new Trail(glm::vec3(primary % 10 * 0.1f, (3 + primary % 10) * 0.1f, (7 + primary % 10) * 0.1f), primary);
	}
}

ComplexBarycenter::ComplexBarycenter(size_t primary, std::vector<size_t> secondaries) {
	this->primary = primary;
	this->secondaries = secondaries;

	bodies[primary]->barycenter = this;
	primaryOrbit = new Trail(bodies[primary]->trail->color, primary);
}

ComplexBarycenter::ComplexBarycenter(std::vector<size_t> indices) {
	double highestMass = 0.0;
	size_t highestMassIndex = -1;
	for (size_t i : indices) {
		if (bodies[i]->mass > highestMass) {
			highestMass = bodies[i]->mass;
			highestMassIndex = i;
		}
	}
	primary = highestMassIndex;
	bodies.erase(bodies.begin() + highestMassIndex);
	secondaries = indices;

	bodies[primary]->barycenter = this;
	primaryOrbit = new Trail(bodies[primary]->trail->color, primary);
}

double ComplexBarycenter::mass(context& context) {
	double mass = context[primary]->mass;
	for (size_t secondary : secondaries)
		mass += context[secondary]->mass;
	return mass;
}

glm::dvec3 ComplexBarycenter::position(context& context) {
	double mass = context[primary]->mass;
	glm::dvec3 massWeightedPos = context[primary]->mass * context[primary]->position;
	for (size_t secondary : secondaries) {
		mass += context[secondary]->mass;
		massWeightedPos += context[secondary]->mass * context[secondary]->position;
	}
	return massWeightedPos / mass;
}

glm::dvec3 ComplexBarycenter::velocity(context& context) {
	double mass = context[primary]->mass;
	glm::dvec3 massWeightedVel = context[primary]->mass * context[primary]->velocity;
	for (size_t secondary : secondaries) {
		mass += context[secondary]->mass;
		massWeightedVel += context[secondary]->mass * context[secondary]->velocity;
	}
	return massWeightedVel / mass;
}

double ComplexBarycenter::apparentMass(context& context, size_t observer) {
	glm::dvec3 netAcceleration(0.0);

	if (observer != primary) {
		glm::dvec3 r = context[primary]->position - context[observer]->position;
		double d = glm::length(r);
		netAcceleration += context[primary]->mass * glm::normalize(r) / (d * d);
	}
	for (size_t secondary : secondaries) {
		if (observer != secondary) {
			glm::dvec3 r = context[secondary]->position - context[observer]->position;
			double d = glm::length(r);
			netAcceleration += context[secondary]->mass * glm::normalize(r) / (d * d);
		}
	}

	glm::dvec3 com = position(context);
	double comDistance = glm::distance(context[observer]->position, com);

	return glm::length(netAcceleration) * comDistance * comDistance;
}

void ComplexBarycenter::positionOffset(context& context, glm::dvec3 offset) {
	context[primary]->position -= offset;
	for (size_t secondary : secondaries)
		context[secondary]->position -= offset;
}

void ComplexBarycenter::velocityOffset(context& context, glm::dvec3 offset) {
	context[primary]->velocity -= offset;
	for (size_t secondary : secondaries)
		context[secondary]->velocity -= offset;
}