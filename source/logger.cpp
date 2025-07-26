#include "logger.h"

std::vector<std::pair<astronomical_data, std::function<void(std::ostream&, char)>>> logTitles = {
    { SEMI_MAJOR_AXIS, [&](std::ostream& os, char dem) { os << "semi-major axis" << dem; } },
    { ECCENTRICITY,    [&](std::ostream& os, char dem) { os << "eccentricity" << dem; } },
    { PERIOD,          [&](std::ostream& os, char dem) { os << "period" << dem; } },
    { ARGUMENT_PERIAPSIS, [&](std::ostream& os, char dem) { os << "arg. of periapsis" << dem; } },
    { ASCENDING_NODE_LONGITUDE, [&](std::ostream& os, char dem) { os << "a.n. of longitude" << dem; } },
    { INCLINATION,     [&](std::ostream& os, char dem) { os << "inclination" << dem; } },
    { MEAN_ANOMALY,    [&](std::ostream& os, char dem) { os << "mean anomaly" << dem; } },
    { MASS,            [&](std::ostream& os, char dem) { os << "mass" << dem; } },
    { RADIUS,          [&](std::ostream& os, char dem) { os << "radius" << dem; } },
    { POSITION,        [&](std::ostream& os, char dem) { os << "position" << dem; } },
    { VELOCITY,        [&](std::ostream& os, char dem) { os << "velocity" << dem; } },
    { ACCELERATION,    [&](std::ostream& os, char dem) { os << "acceleration" << dem; } },
    { TORQUE,          [&](std::ostream& os, char dem) { os << "torque" << dem; } },
};

Logger::Logger(const std::filesystem::path& filePath, size_t body, uint16_t data) {
    this->body = body;
    captureData = data;
    outFile.open(filePath);
    if (!body) {
        fprintf(stderr, "Cannot log data for null GravityBody\n");
        enabled = false;
    }
    if (!outFile.is_open()) {
        fprintf(stderr, "Failed to open log file: %ls\n", filePath.c_str());
        enabled = false;
        return;
    }

    isCSV = filePath.extension() == ".csv";

    outFile << std::setprecision(10) << "time" << (isCSV ? ',' : '\t');

    for (const auto& [flag, handler] : logTitles) {
        if (captureData & flag) {
            handler(outFile, isCSV ? ',' : '\t');
        }
    }

    enabled = true;
}

Logger::~Logger() {
    if (outFile.is_open())
        outFile.close();
}

void Logger::addCondition(logCondition condition) {
	conditions.push_back(condition);
}

void Logger::writeLog(glm::float64 timeStamp) {
    glm::float64 
        semiMajorAxis = 0.0,
        eccentricity = 0.0,
        period = 0.0,
        argPeriapsis = 0.0,
        anLongitude = 0.0,
        inclination = 0.0,
        meanAnomaly = 0.0;

    if (captureData && ORBIT_PROPERTIES) {
        glm::dvec3 r0, r1, velParent, velOrbiter;
        glm::float64 parentMass;

        if (bodies[bodies[body]->parentIndex]->barycenter) {
            // replace parent object parameters with those of its barycenter
            // i.e. a planet with a massive moon where we wish to see the moon's orbit relative to the COM
            Barycenter* bary = bodies[bodies[body]->parentIndex]->barycenter;
            r0 = bary->position(bodies);
            velParent = bary->velocity(bodies);
            parentMass = bary->apparentMass(bodies, body);
        }
        else {
            r0 = bodies[bodies[body]->parentIndex]->position;
            velParent = bodies[bodies[body]->parentIndex]->velocity;
            parentMass = bodies[bodies[body]->parentIndex]->mass;
        }

        if (bodies[body]->barycenter) {
            // replace orbiter object parameters with those of its barycenter
            // i.e. a planet with a massive moon that we wish to track as a single object orbiting a star
            Barycenter* bary = bodies[body]->barycenter;
            r1 = bary->position(bodies);
            velOrbiter = bary->velocity(bodies);
        }
        else {
            r1 = bodies[body]->position;
            velOrbiter = bodies[body]->velocity;
        }

        glm::dvec3 position = r1 - r0;
        glm::dvec3 velocity = velOrbiter - velParent;

        glm::float64 distance = glm::length(position);
        glm::float64 speed = glm::length(velocity);

        glm::float64 gravParam = G * parentMass;

        glm::dvec3 orbitalMomentum = glm::cross(position, velocity);
        glm::dvec3 h_hat = glm::normalize(orbitalMomentum);
        glm::dvec3 eccVector = glm::cross(velocity, orbitalMomentum) / gravParam - position / distance;

        eccentricity = glm::length(eccVector);
        glm::dvec3 e_hat = glm::normalize(eccVector);

        semiMajorAxis = 1.0 / (2.0 / distance - speed * speed / gravParam);

        period = 2 * pi * sqrt(semiMajorAxis * semiMajorAxis * semiMajorAxis / gravParam);

        size_t grandparent_i = bodies[bodies[body]->parentIndex]->parentIndex;
        if (grandparent_i == -1) {
            inclination = acos(orbitalMomentum.y / glm::length(orbitalMomentum));
        }
        else {
            glm::dvec3 gpPos, gpVel;
            glm::float64 gpMass;

            if (bodies[grandparent_i]->barycenter) {
                Barycenter* bary = bodies[grandparent_i]->barycenter;
                gpPos = bary->position(bodies);
                gpVel = bary->velocity(bodies);
                gpMass = bary->apparentMass(bodies, bodies[body]->parentIndex);
            }
            else {
                r0 = bodies[grandparent_i]->position;
                velParent = bodies[grandparent_i]->velocity;
                parentMass = bodies[grandparent_i]->mass;
            }

            glm::dvec3 p2gpPosition = r0 - gpPos;
            glm::dvec3 p2gpVelocity = velParent - gpVel;


            glm::dvec3 parentOrbitalMomentum = glm::cross(p2gpPosition, p2gpVelocity);
            inclination = acos(glm::dot(orbitalMomentum, glm::normalize(parentOrbitalMomentum)) / glm::length(orbitalMomentum));
        }
    }

    std::vector<std::pair<astronomical_data, std::function<void(std::ostream&)>>> logHandlers = {
        { SEMI_MAJOR_AXIS, [&](std::ostream& os) { os << semiMajorAxis << (isCSV ? ',' : '\t'); } },
        { ECCENTRICITY,    [&](std::ostream& os) { os << eccentricity << (isCSV ? ',' : '\t'); } },
        { PERIOD,          [&](std::ostream& os) { os << period << (isCSV ? ',' : '\t'); } },
        { ARGUMENT_PERIAPSIS, [&](std::ostream& os) { os << argPeriapsis << (isCSV ? ',' : '\t'); } },
        { ASCENDING_NODE_LONGITUDE, [&](std::ostream& os) { os << anLongitude << (isCSV ? ',' : '\t'); } },
        { INCLINATION,     [&](std::ostream& os) { os << inclination << (isCSV ? ',' : '\t'); } },
        { MEAN_ANOMALY,    [&](std::ostream& os) { os << meanAnomaly << (isCSV ? ',' : '\t'); } },
        { MASS,            [&](std::ostream& os) { os << bodies[body]->mass << (isCSV ? ',' : '\t'); } },
        { RADIUS,          [&](std::ostream& os) { os << bodies[body]->radius << (isCSV ? ',' : '\t'); } },
        { POSITION,        [&](std::ostream& os) {
            os << bodies[body]->position.x << (isCSV ? ',' : '\t') 
                << bodies[body]->position.y << (isCSV ? ',' : '\t')
                << bodies[body]->position.z << (isCSV ? ',' : '\t');
        }},
        { VELOCITY,        [&](std::ostream& os) {
            os << bodies[body]->velocity.x << (isCSV ? ',' : '\t')
                << bodies[body]->velocity.y << (isCSV ? ',' : '\t')
                << bodies[body]->velocity.z << (isCSV ? ',' : '\t');
        }},
        { ACCELERATION,    [&](std::ostream& os) {
            os << bodies[body]->acceleration.x << (isCSV ? ',' : '\t')
                << bodies[body]->acceleration.y << (isCSV ? ',' : '\t')
                << bodies[body]->acceleration.z << (isCSV ? ',' : '\t');
        }},
        { TORQUE,          [&](std::ostream& os) {
            os << bodies[body]->torque.x << (isCSV ? ',' : '\t')
                << bodies[body]->torque.y << (isCSV ? ',' : '\t')
                << bodies[body]->torque.z << (isCSV ? ',' : '\t');
        }},
    };


    if (captureData)
        outFile << "\n" << timeStamp << (isCSV ? ',' : '\t');

    for (const auto& [flag, handler] : logHandlers) {
        if (captureData & flag) {
            handler(outFile);
        }
    }
}

void Logger::logIfNeeded(glm::float64 timeStamp) {
    if (enabled) {
        for (const logCondition& cond : conditions) {
            if (cond(body)) {
                writeLog(timeStamp);
                break;
            }
        }
    }
}

void Logger::enable() { enabled = true; }
void Logger::disable() { enabled = false; }