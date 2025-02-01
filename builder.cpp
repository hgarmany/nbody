#include "builder.h"
#include "physics.h"

void EntityBuilder::buildSky(size_t modelIndex) {
	std::vector<std::string> faces = {
		"assets/sky/right.jpg",
		"assets/sky/left.jpg",
		"assets/sky/top.jpg",
		"assets/sky/bottom.jpg",
		"assets/sky/front.jpg",
		"assets/sky/back.jpg"
	};

	Surface stars = Surface::CubeMap(faces);

	init();
	setModel(modelIndex);
	setSurface(stars);
	Entity::skybox = get();
}

void GravityBodyBuilder::buildEarthMoonSun(size_t modelIndex, Surface& earth, Surface& moon, Surface& sun) {
	// sun
	init(1.9891e30f);
	setModel(modelIndex);
	setRadius(500.0f);
	double spin = 2 * pi / 86400 / 27;
	setMotion(glm::dvec3(0.0), glm::dvec3(0.0));
	setOrientation(glm::dvec3(0.126, 0, 0));
	setSurface(sun);
	addTrail();
	bodies.push_back(get());

	// earth
	init(5.9722e24f);
	setModel(modelIndex);
	setRadius(250.0f);
	spin = 2 * pi / 86400;
	setMotion(
		glm::dvec3(149598.0, 0.0, 0.0),
		glm::dvec3(0.0, 0.0, 0.029786),
		glm::dvec3(0.0, spin, 0.0)
	);
	setOrientation(glm::dvec3(0.40910518, 0, 0));
	setSurface(earth);
	addTrail(glm::vec3(0.0f, 0.0f, 1.0f));
	bodies.push_back(get());

	//moon
	init(7.3477e22f);
	setModel(modelIndex);
	setRadius(100.0f);
	spin = 2 * pi / 86400 / 27.3;
	setMotion(
		glm::dvec3(149982.7, 0.0, 0.0),
		glm::dvec3(0.0, 0.0, 0.028764),
		glm::dvec3(0.0, spin, 0.0)
	);
	setOrientation(glm::dvec3(0.116588, 0, 0));
	setSurface(moon);
	addTrail();
	bodies.push_back(get());
}