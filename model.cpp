#include "model.h"
#include <unordered_map>
#include <string>

void Model::generateNormals() {
	size_t numFaces = indices.size() / 3;  // Each vertex has 3 components (x, y, z)
	size_t numVerts = vertices.size();

	// Initialize normals to zero
	normals.reserve(numVerts);  // Same size as vertices
	for (int i = 0; i < numVerts; i++) {
		normals.push_back(0.0f);
	}

	// Iterate through each triangle (each group of 3 vertices)
	for (int i = 0; i < numFaces; i++) {
		// Extract the 3 vertices for the current triangle
		GLint a = indices[3 * i];
		GLint b = indices[3 * i + 1];
		GLint c = indices[3 * i + 2];
		glm::vec3 v0(vertices[3 * a], vertices[3 * a + 1], vertices[3 * a + 2]);
		glm::vec3 v1(vertices[3 * b], vertices[3 * b + 1], vertices[3 * b + 2]);
		glm::vec3 v2(vertices[3 * c], vertices[3 * c + 1], vertices[3 * c + 2]);

		// Calculate two edge vectors of the triangle
		glm::vec3 edge1 = v1 - v0;
		glm::vec3 edge2 = v2 - v0;

		// Compute the normal of the triangle (cross product of edges)
		glm::vec3 normal = glm::normalize(glm::cross(glm::normalize(edge1), glm::normalize(edge2)));

		// Add the normal to the normals of the three vertices of this triangle
		normals[3 * a] += normal.x;
		normals[3 * a + 1] += normal.y;
		normals[3 * a + 2] += normal.z;

		normals[3 * b] += normal.x;
		normals[3 * b + 1] += normal.y;
		normals[3 * b + 2] += normal.z;

		normals[3 * c] += normal.x;
		normals[3 * c + 1] += normal.y;
		normals[3 * c + 2] += normal.z;
	}

	// Now normalize the normals for each vertex
	for (size_t i = 0; i < numVerts; i+=3) {
		glm::vec3 normal(normals[i], normals[i + 1], normals[i + 2]);
		normal = glm::normalize(normal);  // Normalize the vector
		normals[i] = normal.x;
		normals[i + 1] = normal.y;
		normals[i + 2] = normal.z;
	}
}

Model Model::Cube() {
	std::vector<GLfloat> tempVer = {
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,

		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f
	};

	std::vector<GLfloat> tempNor = {
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,

		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f
	};

	std::vector<GLfloat> tempTex = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,

		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,

		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,

		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,

		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,

		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f
	};

	std::vector<GLuint> tempInd = {
		0, 2, 1, 0, 3, 2,
		4, 5, 7, 5, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 15, 13, 13, 15, 14,
		16, 17, 19, 17, 18, 19,
		20, 23, 22, 20, 22, 21
	};

	return Model(tempVer, tempInd, tempNor, tempTex);
}

Model Model::Sphere() {
	// Define sphere properties
	const int SPHERE_LATITUDE = 50; // Number of latitude lines
	const int SPHERE_LONGITUDE = 50; // Number of longitude lines

	std::vector<GLfloat> tempVer;
	std::vector<GLuint> tempInd;
	std::vector<GLfloat> tempNor;
	std::vector<GLfloat> tempTex;

	for (int lat = 0; lat <= SPHERE_LATITUDE; ++lat) {
		float theta = lat * pi / SPHERE_LATITUDE; // Latitude angle
		for (int lon = 0; lon <= SPHERE_LONGITUDE; ++lon) {
			float phi = lon * 2.0f * pi / SPHERE_LONGITUDE; // Longitude angle
			float x = sin(theta) * cos(phi);
			float y = cos(theta);
			float z = sin(theta) * sin(phi);

			// Add vertex positions
			tempVer.push_back(x);
			tempVer.push_back(y);
			tempVer.push_back(z);

			// Add normals (normalized position vector)
			tempNor.push_back(x);  // Normal x
			tempNor.push_back(y);  // Normal y
			tempNor.push_back(z);  // Normal z

			// Add texture coordinates (not used for shading but often useful)
			float u = (float)lon / (float)SPHERE_LONGITUDE;
			float v = (float)lat / (float)SPHERE_LATITUDE;
			tempTex.push_back(u);
			tempTex.push_back(v);
		}
	}

	// Generate the indices to form the sphere faces (triangles)
	for (int lat = 0; lat < SPHERE_LATITUDE; lat++) {
		for (int lon = 0; lon < SPHERE_LONGITUDE; lon++) {
			GLuint first = lat * (SPHERE_LONGITUDE + 1) + lon;
			GLuint second = first + SPHERE_LONGITUDE + 1;

			// First triangle
			tempInd.push_back(first);
			tempInd.push_back(second);
			tempInd.push_back(first + 1);

			// Second triangle
			tempInd.push_back(second);
			tempInd.push_back(second + 1);
			tempInd.push_back(first + 1);
		}
	}

	return Model(tempVer, tempInd, tempNor, tempTex);
}

Model Model::Icosphere(int subdivisions) {
	// Define the 12 vertices of the icosahedron
	std::vector<GLfloat> vertices = {
		-.525731f, .0f, .850650f,
		.525731f, .0f, .850650f,
		-.525731f, .0f, -.850650f,
		.525731f, .0f, -.850650f,
		.0f, .850650f, .525731f,
		.0f, .850650f, -.525731f,
		.0f, -.850650f, .525731f,
		.0f, -.850650f, -.525731f,
		.850650f, .525731f, .0f,
		-.850650f, .525731f, .0f,
		.850650f, -.525731f, .0f,
		-.850650f, -.525731f, .0f
	};

	// Initial faces (20 triangles)
	std::vector<std::vector<GLuint>> faces = {
		{0,4,1},{0,9,4},{9,5,4},{4,5,8},{4,8,1},
		{8,10,1},{8,3,10},{5,3,8},{5,2,3},{2,7,3},
		{7,10,3},{7,6,10},{7,11,6},{11,0,6},{0,1,6},
		{6,1,10},{9,0,11},{9,11,2},{9,2,5},{7,2,11}
	};

	// Temporary vectors for the vertex, index, normal, and texture data
	std::vector<GLfloat> tempVer;
	std::vector<GLuint> tempInd;
	std::vector<GLfloat> tempNor;
	std::vector<GLfloat> tempTex;

	// A map to cache the midpoints to avoid duplicating vertices
	std::unordered_map<std::string, GLuint> middleCache;

	// Function to get or create the midpoint between two vertices
	auto getMiddle = [&](GLuint i, GLuint j) -> GLuint {
		if (i > j) std::swap(i, j); // Make sure the smaller index comes first
		std::string key = std::to_string(i) + "-" + std::to_string(j);

		// If the midpoint is already created, return it
		if (middleCache.find(key) != middleCache.end()) {
			return middleCache[key];
		}

		// Calculate the midpoint
		GLfloat x = (vertices[3 * i] + vertices[3 * j]) / 2.0f;
		GLfloat y = (vertices[3 * i + 1] + vertices[3 * j + 1]) / 2.0f;
		GLfloat z = (vertices[3 * i + 2] + vertices[3 * j + 2]) / 2.0f;

		// Normalize the midpoint to the unit sphere
		GLfloat length = sqrt(x * x + y * y + z * z);
		x /= length;
		y /= length;
		z /= length;

		// Add the midpoint as a new vertex
		GLuint newVertex = (GLuint)(vertices.size() / 3);
		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(z);

		// Cache the midpoint index
		middleCache[key] = newVertex;
		return newVertex;
		};

	// Subdivide the faces for the specified number of subdivisions
	for (int sub = 0; sub < subdivisions; ++sub) {
		std::vector<std::vector<GLuint>> newFaces;

		// Loop over the existing faces and subdivide each one
		for (auto& face : faces) {
			GLuint v0 = face[0], v1 = face[1], v2 = face[2];

			// Get the midpoints for the edges of the triangle
			GLuint a = getMiddle(v0, v1);
			GLuint b = getMiddle(v1, v2);
			GLuint c = getMiddle(v2, v0);

			// Create 4 new triangles from the subdivided face
			// Triangle 1: (v0, a, c)
			newFaces.push_back({ v0, a, c });
			// Triangle 2: (v1, b, a)
			newFaces.push_back({ v1, b, a });
			// Triangle 3: (v2, c, b)
			newFaces.push_back({ v2, c, b });
			// Triangle 4: (a, b, c)
			newFaces.push_back({ a, b, c });
		}

		// Update the faces with the new subdivided faces
		faces = std::move(newFaces);
	}

	// Add the vertices, normals, and texture coordinates
	for (size_t i = 0; i < vertices.size(); i += 3) {
		GLfloat x = vertices[i];
		GLfloat y = vertices[i + 1];
		GLfloat z = vertices[i + 2];

		// Normalize the vertex position
		GLfloat length = sqrt(x * x + y * y + z * z);
		GLfloat nx = x / length;
		GLfloat ny = y / length;
		GLfloat nz = z / length;

		// Add the normalized vertex position to the vertex list
		tempVer.push_back(nx);
		tempVer.push_back(ny);
		tempVer.push_back(nz);

		// Normals are the same as the vertex positions for a unit sphere
		tempNor.push_back(nx);
		tempNor.push_back(ny);
		tempNor.push_back(nz);

		// Calculate spherical coordinates (longitude and latitude)
		GLfloat u = 0.5 - (atan2(nz, nx) / (2.0f * pi));
		GLfloat v = 0.5 - (asin(ny) / pi);

		// Add the texture coordinates to the list
		tempTex.push_back(u);
		tempTex.push_back(v);
	}

	// Add the indices for the faces
	for (auto& face : faces) {
		GLfloat u0 = tempTex[2 * face[0]];
		GLfloat u1 = tempTex[2 * face[1]];
		GLfloat u2 = tempTex[2 * face[2]];
		int dupIndex = -1;
		if (abs(u0 - u1) > 0.5f && abs(u0 - u2) > 0.5f) {
			dupIndex = 0;
		}
		else if (abs(u1 - u0) > 0.5f && abs(u1 - u2) > 0.5f) {
			dupIndex = 1;
		}
		else if (abs(u2 - u0) > 0.5f && abs(u2 - u1) > 0.5f) {
			dupIndex = 2;
		}

		if (dupIndex != -1) {
			int newIndex = tempVer.size() / 3;
			tempVer.push_back(tempVer[face[dupIndex] * 3] );
			tempVer.push_back(tempVer[face[dupIndex] * 3 + 1]);
			tempVer.push_back(tempVer[face[dupIndex] * 3 + 2]);

			tempNor.push_back(tempNor[face[dupIndex] * 3]);
			tempNor.push_back(tempNor[face[dupIndex] * 3 + 1]);
			tempNor.push_back(tempNor[face[dupIndex] * 3 + 2]);

			GLfloat oldU = tempTex[face[dupIndex] * 2];
			bool dupIsLeft = oldU < tempTex[face[(dupIndex + 1) % 3]];
			GLfloat newU = dupIsLeft ? oldU + 1.0 : oldU - 1.0;
			tempTex.push_back(newU);
			tempTex.push_back(tempTex[face[dupIndex] * 2 + 1]);

			face[dupIndex] = newIndex;
		}

		tempInd.push_back(face[0]);
		tempInd.push_back(face[1]);
		tempInd.push_back(face[2]);
	}

	// Return the model with vertices, indices, normals, and texture coordinates
	return Model(tempVer, tempInd, tempNor, tempTex);
}
