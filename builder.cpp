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

void GravityBodyBuilder::buildSolarSystem(size_t modelIndex) {
	Surface mercury = Surface("assets/sol/mercury.jpg", glm::vec4(0.0f, 1.0f, 1.0f, 0.0f), glm::vec3(1.0f));
	Surface venus = Surface("assets/sol/venus.jpg", glm::vec4(0.0f, 1.0f, 1.0f, 0.0f), glm::vec3(1.0f));
	Surface earth = Surface("assets/sol/earth.jpg", glm::vec4(0.0f, 1.0f, 1.0f, 0.0f), glm::vec3(1.0f));
	Surface mars = Surface("assets/sol/mars.jpg", glm::vec4(0.0f, 1.0f, 1.0f, 0.0f), glm::vec3(1.0f));
	earth.normal = Surface::getTexture("assets/sol/earth_normal.jpg");
	Surface moon = Surface("assets/sol/moon.jpg", glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), glm::vec3(1.0f));
	moon.normal = Surface::getTexture("assets/sol/moon_normal.jpg");
	Surface sun = Surface("assets/sol/sun.jpg", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec3(1.0f));

	// sun
	init(1.9891e30f);
	setModel(modelIndex);
	setRadius(695.7f);
	double spin = 2 * pi / 86400 / 27;
	setMotion(glm::dvec3(0.0), glm::dvec3(0.0));
	setOrientation(glm::dvec3(0.126, 0, 0));
	setSurface(sun);
	bodies.push_back(get());

	// mercury with true orbital properties
	Orbit mercuryOrbit(&bodies[0], 5.790923e4, 0.20563593f, 1.351894f, 0.843531f, 0.1222599f, 0.0f);
	init(3.301e23f, mercuryOrbit, 0);
	setModel(modelIndex);
	setRadius(2.4397f);
	spin = 2 * pi / 86400 / 58.6;
	setSpin(spin);
	setOrientation(glm::dvec3(0.0005934119 + mercuryOrbit.inclination, 0, 0));
	setSurface(mercury);
	addTrail(glm::vec3(1.0f, 0.0f, 1.0f));
	bodies.push_back(get());

	// venus with true orbital properties
	Orbit venusOrbit(&bodies[0], 1.082095e5, 0.00677672f, 2.296896f, 3.176134f, 0.05924827f, 0.0f);
	init(4.867e24f, venusOrbit, 0);
	setModel(modelIndex);
	setRadius(6.0518f);
	spin = 2 * pi / 86400 / 243;
	setSpin(spin);
	setOrientation(glm::dvec3(0.04607669 + venusOrbit.inclination, 0, 0));
	setSurface(venus);
	addTrail(glm::vec3(1.0f, 1.0f, 0.0f));
	bodies.push_back(get());

	// earth with true orbital properties
	//Orbit earthOrbit(&bodies[0], 1.495983e5, 0.01671123f, 1.796601f, 0.0f, -2.672099e-7f, 1.753438f);
	Orbit earthOrbit(&bodies[0], 1.495983e5, 0.01671123f, 1.796601f, 0.0f, -2.672099e-7f, 0.0f);
	init(5.9722e24f, earthOrbit, 0);
	setModel(modelIndex);
	setRadius(6.371f);
	spin = 2 * pi / 86400;
	setSpin(spin);
	setOrientation(glm::dvec3(0.40910518 + earthOrbit.inclination, 0, 0));
	setSurface(earth);
	addTrail(glm::vec3(0.0f, 0.0f, 1.0f));
	bodies.push_back(get());

	GravityBody* earthPtr = &bodies[3];
	// moon with true orbital properties
	Orbit moonOrbit(earthPtr, 3.84399e2, 0.0549f, 0.0f, 0.0f, 0.08979719f, 0.0f);
	init(5.9722e24f, moonOrbit, 3);
	setModel(modelIndex);
	setRadius(1.7374f);
	spin = 2 * pi / 86400 / 27.321;
	setSpin(spin);
	setOrientation(glm::dvec3(0.02691996 + moonOrbit.inclination, 0, 0));
	setSurface(moon);
	addTrail();
	
	bodies.push_back(get());

	// mars with true orbital properties
	Orbit marsOrbit(&bodies[0], 2.27956e5, 0.09339410f, -0.4178952f, 0.8649771f, 0.03228321f, 0.0f);
	init(6.4169e23f, marsOrbit, 0);
	setModel(modelIndex);
	setRadius(3.3895f);
	spin = 2 * pi / 86400 / 1.029;
	setSpin(spin);
	setOrientation(glm::dvec3(0.4396484 + marsOrbit.inclination, 0, 0));
	setSurface(mars);
	addTrail(glm::vec3(1.0f, 0.0f, 0.0f));
	bodies.push_back(get());
}