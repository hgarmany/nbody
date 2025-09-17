#include "builder.h"

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

void GravityBodyBuilder::buildSolarSystem() {
	size_t sphere = Model::Icosphere(5);
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
	setModel(sphere);
	setRadius(695.7f, 5e-5f);
	double spin = 2 * pi / 86400 / 27;
	setMotion(glm::dvec3(0.0), glm::dvec3(0.0));
	setRotation(glm::dvec3(0.126, 0, 0), glm::dvec3(0, spin, 0));
	setSurface(sun);
	addToBodiesLists();
	
	// mercury
	Orbit mercuryOrbit(5.790923e4, 0.20563593f, 1.351894f, 0.843531f, 0.1222599f, 2.207044f);
	init(3.301e23, mercuryOrbit, 0);
	setModel(sphere);
	setRadius(2.4397f, 9e-4f);
	spin = 2 * pi / 86400 / 58.6;
	setRotation(glm::dvec3(0.0005934119 + mercuryOrbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	setSurface(mercury);
	addTrail(glm::vec3(1.0f, 0.0f, 1.0f));
	addToBodiesLists();

	// venus
	Orbit venusOrbit(1.082095e5, 0.00677672f, 2.296896f, 3.176134f, 0.05924827f, -0.4618222f);
	init(4.867e24, venusOrbit, 0);
	setModel(sphere);
	setRadius(6.0518f);
	spin = -2 * pi / 86400 / 243;
	setRotation(glm::dvec3(0.04607669 + venusOrbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	setSurface(venus);
	addTrail(glm::vec3(1.0f, 1.0f, 0.0f));
	addToBodiesLists();
	
	// earth
	Orbit earthOrbit(1.495983e5, 0.01671123f, 1.796601f, 0.0f, -2.672099e-7f, -0.043163f);
	init(5.9722e24, earthOrbit, 0);
	setModel(sphere);
	setRadius(6.378137f, 3.35e-3f);
	spin = 2 * pi / 86400;
	setRotation(glm::dvec3(0.40910518 + earthOrbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	setSurface(earth);
	addTrail(glm::vec3(0.0f, 0.0f, 1.0f));
	addToBodiesLists();
	
	// mars
	Orbit marsOrbit(2.27956e5, 0.09339410f, -0.4178952f, 0.8649771f, 0.03228321f, -0.5265543f);
	init(6.4169e23, marsOrbit, 0);
	setModel(sphere);
	setRadius(3.3895f, 6.48e-3f);
	spin = 2 * pi / 86400 / 1.029;
	setRotation(glm::dvec3(0.4396484 + marsOrbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	setSurface(mars);
	addTrail(glm::vec3(1.0f, 0.0f, 0.0f));
	addToBodiesLists();

	// jupiter
	Orbit jupiterOrbit(7.783408e5, 0.04838624f, 0.2570605f, 1.753601f, 0.02276602f, -1.412069f);
	init(1.898e27, jupiterOrbit, 0);
	setModel(sphere);
	setRadius(69.911f, 0.06487f);
	spin = 2 * pi / 86400 / 0.415;
	setRotation(glm::dvec3(0.05462881 + jupiterOrbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	setSurface(jupiter);
	addTrail(glm::vec3(1.0f, 0.5f, 0.0f));
	addToBodiesLists();

	// saturn
	Orbit saturnOrbit(1.432041e6, 0.05415060f, 1.613242f, 0.8716928f, 0.04336201f, -2.726251f);
	init(5.6832e26, saturnOrbit, 0);
	setModel(sphere);
	setRadius(60.268f, 0.09796f);
	spin = 2 * pi / 86400 / 0.444;
	setRotation(glm::dvec3(0.4665265 + saturnOrbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	setSurface(saturn);
	addTrail(glm::vec3(0.7f, 0.8f, 0.1f));
	addToBodiesLists();

	// saturn rings
	size_t square = Model::Square();
	glm::vec4 ambMat(1.0f, 0.0f, 0.0f, 0.0f);
	Surface saturn_rings = Surface("../../assets/sol/saturn_rings.png", ambMat);
	EntityBuilder eBuilder;
	eBuilder.init();
	eBuilder.setModel(square);
	eBuilder.setScale(glm::dvec3(139.826));
	eBuilder.setSurface(saturn_rings);
	eBuilder.setRoot(bodies[6]);
	entities.push_back(eBuilder.get());
	
	// moon
	Orbit moonOrbit(384.399, 0.0549f, 0.0f, 0.0f, 0.08979719f, 0.0f);
	init(7.346e22, moonOrbit, 3);
	setModel(sphere);
	setRadius(1.7381f, 1.24e-3f);
	spin = 2 * pi / 86400 / 27.321661;
	setRotation(glm::dvec3(0.02691996 + moonOrbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	setSurface(moon);
	addTrail();
	addToBodiesLists();
	bodies[bodies.size() - 1]->j2 = 2.034e-4; // non-standard j2
	
	// io
	Orbit ioOrbit(421.7, 0.0041f, 1.705798f, 5.462549f, 8.726646e-4f, -5.305661f);
	init(8.932e22, ioOrbit, 5);
	setModel(sphere);
	setRadius(1.8215f);
	spin = 2 * pi / 86400 / 1.769;
	setRotation(glm::dvec3(0.0006981317 + ioOrbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	setSurface(io);
	addTrail(glm::vec3(1.0f, 0.8f, 0.2f));
	addToBodiesLists();

	// europa
	Orbit europaOrbit(670.9, 0.0101f, 2.714196f, 3.078359f, 0.008203047f, -1.400138f);
	init(4.800e22, europaOrbit, 5);
	setModel(sphere);
	setRadius(1.5608f);
	spin = 2 * pi / 86400 / 3.551;
	setRotation(glm::dvec3(0.001745329 + europaOrbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	setSurface(europa);
	addTrail(glm::vec3(0.4f, 0.7f, 0.7f));
	addToBodiesLists();

	// ganymede
	Orbit ganymedeOrbit(1070, 0.0015f, 3.295723f, 2.09162f, 0.003403392f, -3.271899f);
	init(1.4819e23, ganymedeOrbit, 5);
	setModel(sphere);
	setRadius(2.634f);
	spin = 2 * pi / 86400 / 7.155;
	setRotation(glm::dvec3(0.005759587 + ganymedeOrbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	setSurface(ganymede);
	addTrail(glm::vec3(0.0f, 0.4f, 0.8f));
	addToBodiesLists();

	// callisto
	Orbit callistoOrbit(1883, 0.007f, 5.863137f, 5.642039f, 0.004904375f, 2.720846f);
	init(1.0759e23, callistoOrbit, 5);
	setModel(sphere);
	setRadius(2.410f);
	spin = 2 * pi / 86400 / 16.689;
	setRotation(glm::dvec3(0 + callistoOrbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	setSurface(callisto);
	addTrail(glm::vec3(0.6f, 0.6f, 0.8f));
	addToBodiesLists();
}

void GravityBodyBuilder::buildAlienSystem() {
	size_t sphere = Model::Icosphere(5);
	size_t square = Model::Square();
	glm::vec4 diffuseMat(0.0f, 1.0f, 0.0f, 0.0f);
	glm::vec4 ambMat(1.0f, 0.0f, 0.0f, 0.0f);
	Surface sun = Surface("../../assets/sol/sun.jpg", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	Surface world = Surface("../../assets/fiction/world.jpg", diffuseMat);
	Surface earth = Surface("../../assets/sol/earth.jpg", diffuseMat);
	Surface giant_1 = Surface("../../assets/giant_1.jpg", diffuseMat);
	glm::vec4 bdMat(0.0f, 1.0f, 0.0f, 0.04f);
	Surface brown_dwarf = Surface("../../assets/brown_dwarf.jpg", bdMat);
	Surface test = Surface("../../assets/grid.jpg", diffuseMat);
	world.setNormal("../../assets/fiction/world_normal.jpg");

	float siderealDay = 31.7791f * 3600.0f;
	
	// sun
	init(1.76999e30f);
	setModel(sphere);
	setRadius(634.483f, 1.76105e-5f);
	double spin = 2 * pi / 86400 / 27;
	setRotation(glm::dvec3(0.126, 0, 0), glm::dvec3(0, spin, 0));
	setSurface(sun);
	addToBodiesLists();

	// molten
	Orbit moltenOrbit(3.714981e4, 0.066073f, 1.7349201f, 0.2557102f, 0.105305f, -2.510953f);
	init(8.02884e23, moltenOrbit, 0);
	setModel(sphere);
	setRadius(5.9003f, 8.42883e-5f);
	spin = 2 * pi / (54.27202 * siderealDay);
	setRotation(glm::dvec3(0.2596828 + moltenOrbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	Surface mercury = Surface("../../assets/sol/mercury.jpg", diffuseMat);
	//setSurface(Surface(diffuseMat, glm::vec3(0.5f, 0.35f, 0.4f)));
	setSurface(mercury);
	addTrail(glm::vec3(1.0f, 0.0f, 0.0f));
	addToBodiesLists();

	// hothouse
	Orbit hothouseOrbit(9.327584e4, 0.00760225f, 1.633482f, -1.12883f, 0.0161294f, 2.5892111f);
	init(7.6757e24, hothouseOrbit, 0);
	setModel(sphere);
	setRadius(7.1637f, 3.16904e-4f);
	spin = 2 * pi / 419863;
	setRotation(glm::dvec3(0.3759614 + hothouseOrbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	test.color = glm::vec3(0.8f, 0.75f, 0.3f);
	setSurface(test);
	addTrail(glm::vec3(0.8f, 0.5f, 0.5f));
	addToBodiesLists();

	// earth
	Orbit earthOrbit(1.37019e5, 0.0373234f, 0.0f, 0.0f, 0.0f, 0.0f);
	init(2.44536e25, earthOrbit, 0);
	setModel(sphere);
	setRadius(11.3453f, 3.363984e-3f);
	spin = 2 * pi / siderealDay;
	setRotation(glm::dvec3(0.4917866593 + earthOrbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	setSurface(world);
	addTrail(glm::vec3(0.0f, 0.0f, 1.0f));
	addToBodiesLists();

	// !earth
	Orbit nearthOrbit(1.35810e5, 0.0115530f, 0.506823f, 1.0653f, 0.045623f, float(pi));
	init(1.66284e25, nearthOrbit, 0);
	setModel(sphere);
	setRadius(9.6055f, 1.38554e-3f);
	spin = 2 * pi / 394416.71;
	setRotation(glm::dvec3(0.1362159 + nearthOrbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	//setSurface(Surface(diffuseMat, glm::vec3(0.1f, 0.5f, 1.0f)));
	setSurface(earth);
	addTrail(glm::vec3(0.2f, 1.0f, 0.2f));
	addToBodiesLists();

	// hycean
	Orbit hyceanOrbit(6.86212e5, 0.0372854f, -1.061138f, 0.6300549f, 0.04870922f, -0.9925301f);
	init(3.66629e25, hyceanOrbit, 0);
	setModel(sphere);
	setRadius(21.8128f, 0.0736628f);
	spin = -2 * pi / 49207.39;
	setRotation(glm::dvec3(0.1861882 + hyceanOrbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	test.color = glm::vec3(0.7f, 0.78f, 0.87f);
	setSurface(test);
	addTrail(glm::vec3(0.3f, 0.7f, 0.8f));
	addToBodiesLists();

	// ice giant 1
	Orbit iceGiant1Orbit(9.64622e5, 0.0243471f, 2.792516f, 1.10546f, 0.0221453f, -2.251881f);
	init(2.97812e26, iceGiant1Orbit, 0);
	setModel(sphere);
	setRadius(38.702f, 0.130259f);
	spin = 2 * pi / 29478.8;
	setRotation(glm::dvec3(0 + iceGiant1Orbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	setSurface(giant_1);
	addTrail(glm::vec3(0.7f, 0.6f, 0.2f));
	addToBodiesLists();

	// ice giant 2
	Orbit iceGiant2Orbit(1.26401e6, 0.0103693f, -0.611834f, -0.622201f, 0.0500259f, -0.255691f);
	init(1.36479e26, iceGiant2Orbit, 0);
	setModel(sphere);
	setRadius(29.473f, 0.0572661f);
	spin = 2 * pi / 45602.0;
	setRotation(glm::dvec3(0 + iceGiant2Orbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	test.color = glm::vec3(0.2f, 0.44f, 0.88f);
	setSurface(test);
	addTrail(glm::vec3(0.2f, 0.4f, 0.9f));
	addToBodiesLists();

	// distant brown dwarf
	//Orbit dwarfCompanionOrbit(8.608029e6, 0.3942205f, 1.52666f, 2.10449f, 1.132544f, -0.83064f);
	Orbit dwarfCompanionOrbit(8.608029e6, 0.3942205f, 1.52666f, 2.10449f, 0.3424628f, -0.83064f);
	init(5.96162e28, dwarfCompanionOrbit, 0, false);
	setModel(sphere);
	setRadius(68.8499f, 1.000255e-4f);
	spin = 2 * pi / 202176;
	setRotation(glm::dvec3(-0.0509361 + dwarfCompanionOrbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	setSurface(brown_dwarf);
	addTrail(glm::vec3(0.4f, 0.0f, 0.3f));
	addToBodiesLists();

	// brown dwarf planet 1
	Orbit bdPlanetOrbit(9.272375e3, 0.0725011f, 0.16683f, 2.469302f, 0.019753f, -0.837022f);
	init(5.752203e22, bdPlanetOrbit, 8);
	setModel(sphere);
	setRadius(1.916f, 5.36052e-4f);
	spin = 2 * pi / 2812426;
	setRotation(glm::dvec3(0.00199362 + bdPlanetOrbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	test.color = glm::vec3(0.22f, 0.14f, 0.21f);
	setSurface(test);
	addTrail(glm::vec3(1.0f, 1.0f, 1.0f));
	addToBodiesLists();

	// brown dwarf planet 2
	Orbit bdPlanet2Orbit(6.404286e4, 0.015922f, -1.10062f, 2.469302f, 0.0467625f, -2.93860f);
	init(5.752203e22, bdPlanet2Orbit, 8);
	setModel(sphere);
	setRadius(1.916f);
	spin = 0;
	setRotation(glm::dvec3(0.00751066 + bdPlanet2Orbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	test.color = glm::vec3(0.4f, 0.35f, 0.45f);
	setSurface(test);
	addTrail(glm::vec3(1.0f, 1.0f, 1.0f));
	addToBodiesLists();

	// moon
	Orbit moonOrbit(5.00245e2, 0.0233735f, 0.36812f, 1.77315f, 0.145046f, 2.46619f);
	init(2.69171e23, moonOrbit, 3);
	setModel(sphere);
	setRadius(2.72034f, 1.6033e-3f);
	spin = 2 * pi / 1730651;
	setRotation(glm::dvec3(0.0541602 + moonOrbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	//setSurface(Surface(diffuseMat, glm::vec3(0.35f, 0.35f, 0.4f)));
	setSurface(earth);
	addTrail(glm::vec3(0.5f, 0.5f, 0.5f));
	addToBodiesLists();

	// !moon
	//Orbit nmoonOrbit(1.44966e2, 0.1337362f, 2.40028f, 0.55761f, 0.142635f, 1.66287f);
	Orbit nmoonOrbit(1.44966e2, 0.0013374f, 2.40028f, 0.55761f, 0.142635f, 1.66287f);
	init(3.68863e24, nmoonOrbit, 4);
	setModel(sphere);
	setRadius(6.01158f, 5.29033e-4f);
	spin = 2 * pi / 390000;
	setRotation(glm::dvec3(0.1627101 + nmoonOrbit.inclination, 0, 0), glm::dvec3(0, spin, 0));
	//setSurface(Surface(diffuseMat, glm::vec3(0.25f, 0.2f, 0.15f)));
	setSurface(earth);
	addTrail(glm::vec3(0.8f, 0.5f, 0.4f));
	addToBodiesLists();
}

void GravityBodyBuilder::buildTestSystem() {
	size_t sphere = Model::Icosphere(4);

	glm::vec4 ambMat(1.0f, 0.0f, 0.0f, 0.0f);

	init(2e25f);
	setModel(sphere);
	setRadius(50.0f);
	setSurface(Surface(ambMat));
	addToBodiesLists();

	Orbit testOrbit(1e2, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	init(1e25f, testOrbit, 0);
	setModel(sphere);
	setRadius(50.0f);
	setSurface(Surface(ambMat));
	addTrail(glm::vec3(1.0f, 0.0f, 0.0f));
	addToBodiesLists();

	init(1e1f);
	setModel(sphere);
	setRadius(10.0f);
	setSurface(Surface(ambMat));
	addToBodiesLists();
}

// adjust motion of all bodies in the world to achieve net zero motion relative to the world space
static void fixSystemToWorldSpace() {
	glm::dvec3 avgPos(0.0), avgVel(0.0);
	glm::float64 totalMass = 0.0;
	for (const std::shared_ptr<GravityBody>& body : bodies) {
		avgPos += body->position * body->mass;
		avgVel += body->velocity * body->mass;
		totalMass += body->mass;
	}

	avgPos /= totalMass;
	avgVel /= totalMass;

	for (std::shared_ptr<GravityBody>& body : bodies) {
		body->position -= avgPos;
		body->velocity -= avgVel;
	}
}

void buildObjects() {
	// initialize models
	size_t cube = Model::Cube();

	GravityBodyBuilder builder;

	//builder.buildSolarSystem();
	builder.buildAlienSystem();
	//builder.buildTestSystem();

	/*
	// cross section of ring structure
	std::vector<GLfloat> ringCut = {
		-0.2f, 1.5f, 0.0f,
		-0.35f, 1.65f, 0.0f,
		-0.9f, 1.7f, 0.0f,
		-1.0f, 1.75f, 0.0f,
		-0.05f, 1.8f, 0.0f,
		0.05f, 1.8f, 0.0f,
		0.2f, 2.0f, 0.0f,
		0.95f, 3.0f, 0.0f,
		1.0f, 2.95f, 0.0f,
		0.35f, 1.9f, 0.0f,
		0.05f, 1.5f, 0.0f,
		-0.05f, 1.45f, 0.0f,
		-0.05f, -1.45f, 0.0f,
		0.05f, -1.5f, 0.0f,
		0.35f, -1.9f, 0.0f,
		1.0f, -2.95f, 0.0f,
		0.95f, -3.0f, 0.0f,
		0.2f, -2.0f, 0.0f,
		0.05f, -1.8f, 0.0f,
		-0.05f, -1.8f, 0.0f,
		-1.0f, -1.75f, 0.0f,
		-0.9f, -1.7f, 0.0f,
		-0.35f, -1.65f, 0.0f,
		-0.2f, -1.5f, 0.0f
	};

	size_t ring = Model::Ring(ringCut, 900, 5e-2f);

	// ring
	Orbit ringOrbit(1e1, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f);
	builder.init(1e21f, ringOrbit, 3);
	builder.setModel(ring);
	builder.setRadius(1.0f);
	builder.setSurface(Surface(glm::vec4(0.1f, 1.0f, 0.2f, 0.0f), glm::vec3(0.9f, 0.2f, 0.05f)));
	builder.setRotation(glm::dvec3(0.40910518 + ringOrbit.inclination, 0, 0));
	builder.addTrail(glm::vec3(0.0f, 1.0f, 0.0f)); // orbital trail w.r.t earth
	bodies.push_back(builder.get());
	*/

	// camera
	builder.init();
	builder.setRadius(0.0f);
	builder.setMotion(bodies[0]->position + glm::dvec3(0, 0, bodies[0]->radius * 5), bodies[0]->velocity * 1.1);
	builder.addTrail(glm::vec3(1.0f, 1.0f, 0.0f));
	bodies.push_back(builder.get());

	builder.buildSky(cube);

	fixSystemToWorldSpace();

	frameBodies = bodies;
	frameEntities = entities;
}