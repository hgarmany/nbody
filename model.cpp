#include "model.h"
#include <unordered_map>
#include <string>

std::vector<Model> Model::modelLibrary;
std::vector<GLfloat> Model::vertexLibrary;
std::vector<GLuint> Model::indexLibrary;
std::vector<GLfloat> Model::normalLibrary;
std::vector<GLfloat> Model::texLibrary;

Model::Model(
	size_t vertsStart, size_t vertsLength,
	size_t indexStart, size_t indexLength,
	size_t normalStart, size_t normalLength,
	size_t texStart, size_t texLength,
	std::vector<GLfloat>& tan, std::vector<GLfloat>& bitan
) {

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &NorBuf);
	glGenBuffers(1, &TexBuf);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	this->vertsStart = vertsStart;
	this->vertsLength = vertsLength;
	this->indexStart = indexStart;
	this->indexLength = indexLength;

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertsLength * sizeof(GLfloat), &vertexLibrary[vertsStart], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0); // Position

	if (normalLength == 0) {
		normalStart = generateNormals();
		normalLength = vertsLength;
	}

	glBindBuffer(GL_ARRAY_BUFFER, NorBuf);
	glBufferData(GL_ARRAY_BUFFER, normalLength * sizeof(GLfloat), &normalLibrary[normalStart], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0); // Normals

	glBindBuffer(GL_ARRAY_BUFFER, TexBuf);
	glBufferData(GL_ARRAY_BUFFER, texLength * sizeof(GLfloat), &texLibrary[texStart], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0); // Texture coordinates


	if (tan.size() > 0)
	{
		glGenBuffers(1, &TanBuf);
		glBindBuffer(GL_ARRAY_BUFFER, TanBuf);
		glBufferData(GL_ARRAY_BUFFER, tan.size() * sizeof(GLfloat), &tan[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0); // Normals
	}

	if (bitan.size() > 0)
	{
		glGenBuffers(1, &BitanBuf);
		glBindBuffer(GL_ARRAY_BUFFER, BitanBuf);
		glBufferData(GL_ARRAY_BUFFER, bitan.size() * sizeof(GLfloat), &bitan[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0); // Normals
	}


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexLength * sizeof(GLuint), &indexLibrary[indexStart], GL_STATIC_DRAW);

	glBindVertexArray(0);
}

Model::Model(
	size_t vertsStart, size_t vertsLength,
	size_t indexStart, size_t indexLength,
	size_t normalStart, size_t normalLength,
	size_t texStart, size_t texLength
) {

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &NorBuf);
	glGenBuffers(1, &TexBuf);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	this->vertsStart = vertsStart;
	this->vertsLength = vertsLength;
	this->indexStart = indexStart;
	this->indexLength = indexLength;

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertsLength * sizeof(GLfloat), &vertexLibrary[vertsStart], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0); // Position

	if (normalLength == 0) {
		normalStart = generateNormals();
		normalLength = vertsLength;
	}

	glBindBuffer(GL_ARRAY_BUFFER, NorBuf);
	glBufferData(GL_ARRAY_BUFFER, normalLength * sizeof(GLfloat), &normalLibrary[normalStart], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0); // Normals

	glBindBuffer(GL_ARRAY_BUFFER, TexBuf);
	glBufferData(GL_ARRAY_BUFFER, texLength * sizeof(GLfloat), &texLibrary[texStart], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0); // Texture coordinates

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexLength * sizeof(GLuint), &indexLibrary[indexStart], GL_STATIC_DRAW);

	glBindVertexArray(0);
}

size_t Model::ModelFromImportedVectors(
	std::vector<GLfloat>& verts, 
	std::vector<GLuint>& indices, 
	std::vector<GLfloat>& normals, 
	std::vector<GLfloat>& tex,
	std::vector<GLfloat>& tan,
	std::vector<GLfloat>& bitan) {

	size_t vertStart = vertexLibrary.size();
	size_t indexStart = indexLibrary.size();
	size_t normalStart = normalLibrary.size();
	size_t texStart = texLibrary.size();

	vertexLibrary.insert(vertexLibrary.end(), verts.begin(), verts.end());
	indexLibrary.insert(indexLibrary.end(), indices.begin(), indices.end());
	normalLibrary.insert(normalLibrary.end(), normals.begin(), normals.end());
	texLibrary.insert(texLibrary.end(), tex.begin(), tex.end());

	modelLibrary.emplace_back(
		vertStart, verts.size(), 
		indexStart, indices.size(), 
		normalStart, normals.size(), 
		texStart, tex.size(),
		tan, bitan);
	return modelLibrary.size() - 1;
}

size_t Model::ModelFromImportedVectors(std::vector<GLfloat>& verts, std::vector<GLuint>& indices, std::vector<GLfloat>& normals, std::vector<GLfloat>& tex) {
	size_t vertStart = vertexLibrary.size();
	size_t indexStart = indexLibrary.size();
	size_t normalStart = normalLibrary.size();
	size_t texStart = texLibrary.size();

	vertexLibrary.insert(vertexLibrary.end(), verts.begin(), verts.end());
	indexLibrary.insert(indexLibrary.end(), indices.begin(), indices.end());
	normalLibrary.insert(normalLibrary.end(), normals.begin(), normals.end());
	texLibrary.insert(texLibrary.end(), tex.begin(), tex.end());

	modelLibrary.emplace_back(vertStart, verts.size(), indexStart, indices.size(), normalStart, normals.size(), texStart, tex.size());
	return modelLibrary.size() - 1;
}

size_t Model::generateNormals() {
	size_t numFaces = indexLength / 3;  // Each vertex has 3 components (x, y, z)
	size_t numVerts = vertsLength;

	// Initialize normals to zero
	std::vector<GLfloat> normals;
	normals.reserve(numVerts);  // Same size as vertices
	for (int i = 0; i < numVerts; i++) {
		normals.push_back(0.0f);
	}

	// Iterate through each triangle (each group of 3 vertices)
	for (int i = 0; i < numFaces; i++) {
		// Extract the 3 vertices for the current triangle
		GLint a = indexLibrary[indexStart + 3 * i];
		GLint b = indexLibrary[indexStart + 3 * i + 1];
		GLint c = indexLibrary[indexStart + 3 * i + 2];
		glm::vec3 v0(vertexLibrary[vertsStart + 3 * a], vertexLibrary[vertsStart + 3 * a + 1], vertexLibrary[vertsStart + 3 * a + 2]);
		glm::vec3 v1(vertexLibrary[vertsStart + 3 * b], vertexLibrary[vertsStart + 3 * b + 1], vertexLibrary[vertsStart + 3 * b + 2]);
		glm::vec3 v2(vertexLibrary[vertsStart + 3 * c], vertexLibrary[vertsStart + 3 * c + 1], vertexLibrary[vertsStart + 3 * c + 2]);

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

	size_t normalStart = normalLibrary.size();
	normalLibrary.insert(normalLibrary.end(), normals.begin(), normals.end());
	return normalStart;
}

size_t duplicatePoint(std::vector<GLfloat>& vertices, std::vector<GLfloat>& normals, std::vector<GLfloat>& tex, size_t index) {
	size_t newIndex = vertices.size() / 3;

	vertices.push_back(vertices[index * 3]);
	vertices.push_back(vertices[index * 3 + 1]);
	vertices.push_back(vertices[index * 3 + 2]);

	normals.push_back(normals[index * 3]);
	normals.push_back(normals[index * 3 + 1]);
	normals.push_back(normals[index * 3 + 2]);

	tex.push_back(tex[index * 2]);
	tex.push_back(tex[index * 2 + 1]);

	return newIndex;
}

size_t Model::Cube() {
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

	return ModelFromImportedVectors(tempVer, tempInd, tempNor, tempTex);
}

size_t Model::Sphere() {
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

	return ModelFromImportedVectors(tempVer, tempInd, tempNor, tempTex);
}

size_t Model::Icosphere(int subdivisions) {
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
		GLfloat u = 0.5f - (atan2(nz, nx) / (2.0f * pi));
		GLfloat v = 0.5f - (asin(ny) / pi);

		// Add the texture coordinates to the list
		tempTex.push_back(u);
		tempTex.push_back(v);
	}

	// Add the indices for the faces
	for (auto& face : faces) {
		GLfloat u0 = tempTex[face[0] * 2];
		GLfloat u1 = tempTex[face[1] * 2];
		GLfloat u2 = tempTex[face[2] * 2];


		// fix polar stitching
		int dupIndex = -1;
		if (abs(tempVer[face[0] * 3 + 1]) > 0.999f)
			dupIndex = 0;
		else if (abs(tempVer[face[1] * 3 + 1]) > 0.999f)
			dupIndex = 1;
		else if (abs(tempVer[face[2] * 3 + 1]) > 0.999f)
			dupIndex = 2;

		if (dupIndex != -1) {
			GLuint v0 = face[dupIndex];
			GLuint v1 = face[(dupIndex + 1) % 3];
			GLuint v2 = face[(dupIndex + 2) % 3];

			size_t poleVert = duplicatePoint(tempVer, tempNor, tempTex, v0);
			tempTex[poleVert * 2] = (tempTex[v1 * 2] + tempTex[v2 * 2]) * 0.5f;
			v0 = face[dupIndex] = poleVert;
			printf("%.3f\t%.3f\t%.3f\n", tempTex[v0 * 2], tempTex[v1 * 2], tempTex[v2 * 2]);

			if (abs(tempTex[v1 * 2] - tempTex[v2 * 2]) > 0.5f) {
				printf("split\n");
				size_t middleVert = tempVer.size() / 3;
				tempVer.push_back((tempVer[v1 * 3] + tempVer[v2 * 3]) * 0.5f);
				tempVer.push_back((tempVer[v1 * 3 + 1] + tempVer[v2 * 3 + 1]) * 0.5f);
				tempVer.push_back((tempVer[v2 * 3 + 2] + tempVer[v2 * 3 + 2]) * 0.5f);

				tempNor.push_back(tempVer[middleVert * 3]);
				tempNor.push_back(tempVer[middleVert * 3 + 1]);
				tempNor.push_back(tempVer[middleVert * 3 + 2]);

				tempTex.push_back(0.0f);
				tempTex.push_back(tempTex[v1 * 2 + 1]);

				tempTex[poleVert * 2] = 0.0f;
				size_t poleVertCopy = duplicatePoint(tempVer, tempNor, tempTex, poleVert);
				size_t middleVertCopy = duplicatePoint(tempVer, tempNor, tempTex, middleVert);
				tempTex[poleVertCopy * 2] = 1.0f;
				tempTex[middleVertCopy * 2] = 1.0f;

				if (tempTex[v1 * 2] > 0.5f) {
					tempInd.push_back(poleVertCopy);
					tempInd.push_back(v1);
					tempInd.push_back(middleVertCopy);
					printf("%.3f\t%.3f\t%.3f\n", tempTex[poleVertCopy * 2], tempTex[v1 * 2], tempTex[middleVertCopy * 2]);

					face[0] = poleVert;
					face[1] = middleVert;
					face[2] = v2;
					printf("%.3f\t%.3f\t%.3f\n", tempTex[poleVert * 2], tempTex[middleVert * 2], tempTex[v2 * 2]);
				}
				else {
					tempInd.push_back(poleVertCopy);
					tempInd.push_back(middleVertCopy);
					tempInd.push_back(v2);
					printf("%.3f\t%.3f\t%.3f\n", tempTex[poleVertCopy * 2], tempTex[middleVertCopy * 2], tempTex[v2 * 2]);

					face[0] = poleVert;
					face[1] = v1;
					face[2] = middleVert;
					printf("%.3f\t%.3f\t%.3f\n", tempTex[poleVert * 2], tempTex[v1 * 2], tempTex[middleVert * 2]);
				}
			}
		}

		// fix longitudinal seam
		dupIndex = -1;
		if (abs(u0 - u1) > 0.5f && abs(u0 - u2) > 0.5f)
			dupIndex = 0;
		else if (abs(u1 - u0) > 0.5f && abs(u1 - u2) > 0.5f)
			dupIndex = 1;
		else if (abs(u2 - u0) > 0.5f && abs(u2 - u1) > 0.5f)
			dupIndex = 2;

		if (dupIndex != -1) {
			size_t newIndex = duplicatePoint(tempVer, tempNor, tempTex, face[dupIndex]);

			GLfloat oldU = tempTex[face[dupIndex] * 2];
			bool dupIsLeft = oldU < tempTex[face[(dupIndex + 1) % 3]];
			GLfloat newU = dupIsLeft ? oldU + 1.0f : oldU - 1.0f;
			tempTex[newIndex * 2] = newU;
			tempTex[newIndex * 2 + 1] = tempTex[face[dupIndex] * 2 + 1];

			face[dupIndex] = (GLuint)newIndex;
		}

		/*
		if (tempVer[face[0] * 3] == 0.0f && tempVer[face[0] * 3 + 2] == 0.0f) {
			printf("0 %u: %.2f\n", face[0], tempTex[face[0]]);
		}
		if (tempVer[face[1] * 3] == 0.0f && tempVer[face[1] * 3 + 2] == 0.0f) {
			printf("1 %u: %.2f\n", face[1], tempTex[face[1]]);
		}
		if (tempVer[face[2] * 3] == 0.0f && tempVer[face[2] * 3 + 2] == 0.0f) {
			printf("2 %u: %.2f\n", face[2], tempTex[face[2]]);
		}
		*/

		/*
		GLuint V0start = face[0] * 3;
		if (tempVer[V0start] == 0.0f && tempVer[V0start + 2] == 0.0f) {
			size_t newVert = duplicatePoint(tempVer, tempNor, tempTex, face[0]);
			tempTex[newVert * 2] = (tempTex[face[1] * 2] + tempTex[face[2] * 2]) * 0.5f;
			tempTex[newVert * 2 + 1] = tempVer[newVert * 2 + 1] > 0 ? 0.0f : 1.0f;
			face[0] = newVert;
		}
		*/
		/*
		if (tempVer[V0start] == 0.0f && tempVer[V0start + 2] == 0.0f &&
			abs(tempTex[face[1] * 2] - tempTex[face[2] * 2]) > 0.5)
		{
			printf("%u\n", face[0]);
			size_t copyV0 = duplicatePoint(tempVer, tempNor, tempTex, face[0]);

			// make vertex halfway between either non-pole vertex
			size_t middleVert = tempVer.size() / 3;

			tempVer.push_back((tempVer[face[1] * 3] + tempVer[face[2] * 3]) * 0.5f);
			tempVer.push_back((tempVer[face[1] * 3 + 1] + tempVer[face[2] * 3 + 1]) * 0.5f);
			tempVer.push_back((tempVer[face[1] * 3 + 2] + tempVer[face[2] * 3 + 2]) * 0.5f);

			tempNor.push_back((tempNor[face[1] * 3] + tempNor[face[2] * 3]) * 0.5f);
			tempNor.push_back((tempNor[face[1] * 3 + 1] + tempNor[face[2] * 3 + 1]) * 0.5f);
			tempNor.push_back((tempNor[face[1] * 3 + 2] + tempNor[face[2] * 3 + 2]) * 0.5f);

			tempTex.push_back(1.0f);
			tempTex.push_back(tempTex[face[1] * 2 + 1]);

			size_t dupMiddleVert = duplicatePoint(tempVer, tempNor, tempTex, middleVert);

			tempTex[face[0] * 2] = 1.0f;
			tempTex[copyV0 * 2] = 0.0f;
			tempTex[dupMiddleVert * 2] = 0.0f;

			if (tempTex[face[1] * 2] > 0.5) {
				tempInd.push_back(copyV0);
				tempInd.push_back(dupMiddleVert);
				tempInd.push_back(face[2]);

				face[2] = middleVert;
			}
			else {
				tempInd.push_back(copyV0);
				tempInd.push_back(face[1]);
				tempInd.push_back(dupMiddleVert);

				face[1] = middleVert;
			}
		}
		*/
		tempInd.push_back(face[0]);
		tempInd.push_back(face[1]);
		tempInd.push_back(face[2]);
	}

	// Add tangent and bitangent storage
	std::vector<GLfloat> tempTan(tempVer.size(), 0.0f);
	std::vector<GLfloat> tempBitan(tempVer.size(), 0.0f);

	// Compute tangents and bitangents
	for (size_t i = 0; i < tempInd.size(); i += 3) {
		GLuint i0 = tempInd[i];
		GLuint i1 = tempInd[i + 1];
		GLuint i2 = tempInd[i + 2];

		// Get vertex positions
		glm::vec3 v0(tempVer[i0 * 3], tempVer[i0 * 3 + 1], tempVer[i0 * 3 + 2]);
		glm::vec3 v1(tempVer[i1 * 3], tempVer[i1 * 3 + 1], tempVer[i1 * 3 + 2]);
		glm::vec3 v2(tempVer[i2 * 3], tempVer[i2 * 3 + 1], tempVer[i2 * 3 + 2]);

		// Get texture coordinates
		glm::vec2 uv0(tempTex[i0 * 2], tempTex[i0 * 2 + 1]);
		glm::vec2 uv1(tempTex[i1 * 2], tempTex[i1 * 2 + 1]);
		glm::vec2 uv2(tempTex[i2 * 2], tempTex[i2 * 2 + 1]);

		// Calculate edges and delta UVs
		glm::vec3 edge1 = v1 - v0;
		glm::vec3 edge2 = v2 - v0;
		glm::vec2 deltaUV1 = uv1 - uv0;
		glm::vec2 deltaUV2 = uv2 - uv0;

		// Calculate the tangent and bitangent
		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		glm::vec3 tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
		glm::vec3 bitangent = f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);

		// Accumulate tangents and bitangents for each vertex
		for (GLuint idx : {i0, i1, i2}) {
			tempTan[idx * 3] += tangent.x;
			tempTan[idx * 3 + 1] += tangent.y;
			tempTan[idx * 3 + 2] += tangent.z;

			tempBitan[idx * 3] += bitangent.x;
			tempBitan[idx * 3 + 1] += bitangent.y;
			tempBitan[idx * 3 + 2] += bitangent.z;
		}
	}

	// Normalize tangents and orthogonalize
	for (size_t i = 0; i < tempVer.size(); i += 3) {
		glm::vec3 normal(tempNor[i], tempNor[i + 1], tempNor[i + 2]);
		glm::vec3 tangent(tempTan[i], tempTan[i + 1], tempTan[i + 2]);

		// Orthogonalize tangent
		tangent = glm::normalize(tangent - normal * glm::dot(normal, tangent));

		// Recompute bitangent to ensure consistency
		glm::vec3 bitangent = glm::cross(normal, tangent);

		// Store the results back
		tempTan[i] = tangent.x;
		tempTan[i + 1] = tangent.y;
		tempTan[i + 2] = tangent.z;

		tempBitan[i] = bitangent.x;
		tempBitan[i + 1] = bitangent.y;
		tempBitan[i + 2] = bitangent.z;
	}

	// Pass tangent and bitangent data to your model creation
	return ModelFromImportedVectors(tempVer, tempInd, tempNor, tempTex, tempTan, tempBitan);


	//return ModelFromImportedVectors(tempVer, tempInd, tempNor, tempTex);
}
