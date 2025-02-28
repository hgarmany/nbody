#include "builder.h"
#include "physics.h"

void EntityBuilder::buildSky(size_t modelIndex) {
	std::vector<std::string> faces = {
		"../../assets/sky/right.jpg",
		"../../assets/sky/left.jpg",
		"../../assets/sky/top.jpg",
		"../../assets/sky/bottom.jpg",
		"../../assets/sky/front.jpg",
		"../../assets/sky/back.jpg"
	};

	Surface stars = Surface::CubeMap(faces);

	init();
	setModel(modelIndex);
	setSurface(stars);
	Entity::skybox = get();
}

void GravityBodyBuilder::buildSolarSystem(size_t modelIndex) {
	glm::vec4 diffuseMat(0.0f, 1.0f, 0.0f, 0.0f);
	Surface sun = Surface("../../assets/sol/sun.jpg", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	Surface mercury = Surface("../../assets/sol/mercury.jpg", diffuseMat);
	Surface venus = Surface("../../assets/sol/venus.jpg", diffuseMat);
	Surface earth = Surface("../../assets/sol/earth.jpg", diffuseMat);
	earth.setNormal("../../assets/sol/earth_normal.jpg");
	Surface moon = Surface("../../assets/sol/moon.jpg", diffuseMat);
	moon.setNormal("../../assets/sol/moon_normal.jpg");
	Surface mars = Surface("../../assets/sol/mars.jpg", diffuseMat);
	mars.setNormal("../../assets/sol/mars_normal.jpg");
	Surface jupiter = Surface("../../assets/sol/jupiter.jpg", diffuseMat);
	Surface io = Surface("../../assets/sol/io.jpg", diffuseMat);
	io.setNormal("../../assets/sol/io_normal.jpg");
	Surface europa = Surface("../../assets/sol/europa.jpg", diffuseMat);
	Surface ganymede = Surface("../../assets/sol/ganymede.jpg", diffuseMat);
	Surface callisto = Surface("../../assets/sol/callisto.jpg", diffuseMat);
	callisto.setNormal("../../assets/sol/callisto_normal.jpg");
	Surface saturn = Surface("../../assets/sol/saturn.jpg", diffuseMat);

	// sun
	init(1.9891e30f);
	setModel(modelIndex);
	setRadius(695.7f, 5e-5f);
	double spin = 2 * pi / 86400 / 27;
	setMotion(glm::dvec3(0.0), glm::dvec3(0.0));
	setSpin(spin);
	setOrientation(glm::dvec3(0.126, 0, 0));
	setSurface(sun);
	bodies.push_back(get());

	// mercury
	Orbit mercuryOrbit(&bodies[0], 5.790923e4, 0.20563593f, 1.351894f, 0.843531f, 0.1222599f, 2.207044f);
	init(3.301e23, mercuryOrbit, 0);
	setModel(modelIndex);
	setRadius(2.4397f, 9e-4f);
	spin = 2 * pi / 86400 / 58.6;
	setSpin(spin);
	setOrientation(glm::dvec3(0.0005934119 + mercuryOrbit.inclination, 0, 0));
	setSurface(mercury);
	addTrail(glm::vec3(1.0f, 0.0f, 1.0f));
	bodies.push_back(get());

	// venus
	Orbit venusOrbit(&bodies[0], 1.082095e5, 0.00677672f, 2.296896f, 3.176134f, 0.05924827f, -0.4618222f);
	init(4.867e24, venusOrbit, 0);
	setModel(modelIndex);
	setRadius(6.0518f);
	spin = 2 * pi / 86400 / 243;
	setSpin(spin);
	setOrientation(glm::dvec3(0.04607669 + venusOrbit.inclination, 0, 0));
	setSurface(venus);
	addTrail(glm::vec3(1.0f, 1.0f, 0.0f));
	bodies.push_back(get());

	// earth
	Orbit earthOrbit(&bodies[0], 1.495983e5, 0.01671123f, 1.796601f, 0.0f, -2.672099e-7f, -0.043163f);
	init(5.9722e24, earthOrbit, 0);
	setModel(modelIndex);
	setRadius(6.378137f, 3.35e-3f);
	spin = 2 * pi / 86400;
	setSpin(spin);
	setOrientation(glm::dvec3(0.40910518 + earthOrbit.inclination, 0, 0));
	setSurface(earth);
	addTrail(glm::vec3(0.0f, 0.0f, 1.0f));
	bodies.push_back(get());

	// mars
	Orbit marsOrbit(&bodies[0], 2.27956e5, 0.09339410f, -0.4178952f, 0.8649771f, 0.03228321f, -0.5265543f);
	init(6.4169e23, marsOrbit, 0);
	setModel(modelIndex);
	setRadius(3.3895f, 6.48e-3f);
	spin = 2 * pi / 86400 / 1.029;
	setSpin(spin);
	setOrientation(glm::dvec3(0.4396484 + marsOrbit.inclination, 0, 0));
	setSurface(mars);
	addTrail(glm::vec3(1.0f, 0.0f, 0.0f));
	bodies.push_back(get());

	// jupiter
	Orbit jupiterOrbit(&bodies[0], 7.783408e5, 0.04838624f, 0.2570605f, 1.753601f, 0.02276602f, -1.412069f);
	init(1.898e27, jupiterOrbit, 0);
	setModel(modelIndex);
	setRadius(69.911f, 0.06487f);
	spin = 2 * pi / 86400 / 0.415;
	setSpin(spin);
	setOrientation(glm::dvec3(0.05462881 + jupiterOrbit.inclination, 0, 0));
	setSurface(jupiter);
	addTrail(glm::vec3(1.0f, 0.5f, 0.0f));
	bodies.push_back(get());

	// saturn
	Orbit saturnOrbit(&bodies[0], 1.432041e6, 0.05415060f, 1.613242f, 0.8716928f, 0.04336201f, -2.726251f);
	init(5.6832e26, saturnOrbit, 0);
	setModel(modelIndex);
	setRadius(60.268f, 0.09796f);
	spin = 2 * pi / 86400 / 0.444;
	setSpin(spin);
	setOrientation(glm::dvec3(0.4665265 + saturnOrbit.inclination, 0, 0));
	setSurface(saturn);
	addTrail(glm::vec3(0.7f, 0.8f, 0.1f));
	bodies.push_back(get());

	// moon
	Orbit moonOrbit(&bodies[3], 384.399, 0.0549f, 0.0f, 0.0f, 0.08979719f, 0.0f);
	init(5.9722e24, moonOrbit, 3);
	setModel(modelIndex);
	setRadius(1.7374f, 1.2e-3f);
	spin = 2 * pi / 86400 / 27.321;
	setSpin(spin);
	setOrientation(glm::dvec3(0.02691996 + moonOrbit.inclination, 0, 0));
	setSurface(moon);
	addTrail();

	bodies.push_back(get());

	// io
	Orbit ioOrbit(&bodies[5], 421.7, 0.0041f, 1.705798f, 5.462549f, 8.726646e-4f, -5.305661f);
	init(8.932e22, ioOrbit, 5);
	setModel(modelIndex);
	setRadius(1.8215f);
	spin = 2 * pi / 86400 / 1.769;
	setSpin(spin);
	setOrientation(glm::dvec3(0.0006981317 + ioOrbit.inclination, 0, 0));
	setSurface(io);
	addTrail(glm::vec3(1.0f, 0.8f, 0.2f));
	bodies.push_back(get());

	// europa
	Orbit europaOrbit(&bodies[5], 670.9, 0.0101f, 2.714196f, 3.078359f, 0.008203047f, -1.400138f);
	init(4.800e22, europaOrbit, 5);
	setModel(modelIndex);
	setRadius(1.5608f);
	spin = 2 * pi / 86400 / 3.551;
	setSpin(spin);
	setOrientation(glm::dvec3(0.001745329 + europaOrbit.inclination, 0, 0));
	setSurface(europa);
	addTrail(glm::vec3(0.4f, 0.7f, 0.7f));
	bodies.push_back(get());

	// ganymede
	Orbit ganymedeOrbit(&bodies[5], 1070, 0.0015f, 3.295723f, 2.09162f, 0.003403392f, -3.271899f);
	init(1.4819e23, ganymedeOrbit, 5);
	setModel(modelIndex);
	setRadius(2.634f);
	spin = 2 * pi / 86400 / 7.155;
	setSpin(spin);
	setOrientation(glm::dvec3(0.005759587 + ganymedeOrbit.inclination, 0, 0));
	setSurface(ganymede);
	addTrail(glm::vec3(0.0f, 0.4f, 0.8f));
	bodies.push_back(get());

	// callisto
	Orbit callistoOrbit(&bodies[5], 1883, 0.007f, 5.863137f, 5.642039f, 0.004904375f, 2.720846f);
	init(1.0759e23, callistoOrbit, 5);
	setModel(modelIndex);
	setRadius(2.410f);
	spin = 2 * pi / 86400 / 16.689;
	setSpin(spin);
	setOrientation(glm::dvec3(0 + callistoOrbit.inclination, 0, 0));
	setSurface(callisto);
	addTrail(glm::vec3(0.6f, 0.6f, 0.8f));
	bodies.push_back(get());
}