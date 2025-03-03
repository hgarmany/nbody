#include "model.h"
#include <unordered_map>
#include <string>
#include <fstream>

std::vector<Model> Model::modelLibrary;

void toOBJ(std::vector<GLfloat>& vertices, std::vector<GLuint>& faces) {
	FILE* out;
	fopen_s(&out, "test.obj", "w");

	for (size_t i = 0; i < vertices.size(); i += 3) {
		fprintf(out, "v %.4f %.4f %.4f\n", vertices[i], vertices[i + 1], vertices[i + 2]);
	}
	for (size_t i = 0; i < faces.size(); i += 3) {
		fprintf(out, "f %u %u %u\n", faces[i] + 1, faces[i + 1] + 1, faces[i + 2] + 1);
	}

	fclose(out);
}

Model::Model(
	std::vector<GLfloat>& verts, std::vector<GLuint>& indices,
	std::vector<GLfloat>& normals, std::vector<GLfloat>& tex,
	std::vector<GLfloat>& tan, std::vector<GLfloat>& bitan
) {

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &NorBuf);
	glGenBuffers(1, &TexBuf);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	// load vertex data
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat), verts.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0); // Position

	// load normals, generating new ones if they are unavailable
	if (normals.size() == 0) {
		normals = generateNormals(verts, indices);
	}

	glBindBuffer(GL_ARRAY_BUFFER, NorBuf);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0); // Normals

	// load uv mapping
	glBindBuffer(GL_ARRAY_BUFFER, TexBuf);
	glBufferData(GL_ARRAY_BUFFER, tex.size() * sizeof(GLfloat), tex.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0); // Texture coordinates

	// load tangent and bitangent data if they exist
	if (tan.size() > 0) {
		glGenBuffers(1, &TanBuf);
		glBindBuffer(GL_ARRAY_BUFFER, TanBuf);
		glBufferData(GL_ARRAY_BUFFER, tan.size() * sizeof(GLfloat), tan.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	if (bitan.size() > 0) {
		glGenBuffers(1, &BitanBuf);
		glBindBuffer(GL_ARRAY_BUFFER, BitanBuf);
		glBufferData(GL_ARRAY_BUFFER, bitan.size() * sizeof(GLfloat), bitan.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

	numFaces = (GLsizei)indices.size();
}

Model::Model(
	std::vector<GLfloat>& verts, std::vector<GLuint>& indices,
	std::vector<GLfloat>& normals, std::vector<GLfloat>& tex
) {

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &NorBuf);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat), verts.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0); // Position

	if (normals.size() == 0) {
		normals = generateNormals(verts, indices);
	}

	glBindBuffer(GL_ARRAY_BUFFER, NorBuf);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0); // Normals

	if (tex.size()) {
		glGenBuffers(1, &TexBuf);
		glBindBuffer(GL_ARRAY_BUFFER, TexBuf);
		glBufferData(GL_ARRAY_BUFFER, tex.size() * sizeof(GLfloat), tex.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0); // Texture coordinates
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

	numFaces = (GLsizei)indices.size();
}

std::vector<GLfloat> Model::generateNormals(std::vector<GLfloat>& verts, std::vector<GLuint>& indices) {
	size_t numFaces = indices.size() / 3;  // Each vertex has 3 components (x, y, z)
	size_t numVerts = verts.size();

	// Initialize normals to zero
	std::vector<GLfloat> normals;
	normals.reserve(numVerts);  // Same size as vertices
	for (int i = 0; i < numVerts; i++) {
		normals.push_back(0.0f);
	}

	// Iterate through each triangle (each group of 3 vertices)
	for (int i = 0; i < numFaces; i++) {
		// Extract the 3 vertices for the current triangle
		GLint a = indices[i * 3];
		GLint b = indices[i * 3 + 1];
		GLint c = indices[i * 3 + 2];
		
		glm::vec3 v0(verts[a * 3], verts[a * 3 + 1], verts[a * 3 + 2]);
		glm::vec3 v1(verts[b * 3], verts[b * 3 + 1], verts[b * 3 + 2]);
		glm::vec3 v2(verts[c * 3], verts[c * 3 + 1], verts[c * 3 + 2]);

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

	return normals;
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

	modelLibrary.emplace_back(tempVer, tempInd, tempNor, tempTex);
	return modelLibrary.size() - 1;
}

void normalize(GLfloat& x, GLfloat& y, GLfloat& z) {
	float length = std::sqrt(x * x + y * y + z * z);
	x /= length;
	y /= length;
	z /= length;
}

// Relaxation algorithm for vertices
void relax_vertices(std::vector<GLfloat>& vertices, std::vector<GLuint>& faces, int iterations = 10) {
	for (int iter = 0; iter < iterations; ++iter) {
		std::vector<GLfloat> new_positions(vertices.size(), 0.0f);
		std::vector<int> counts(vertices.size() / 3, 0);

		// Average neighbor positions
		for (int j = 0; j < faces.size(); j += 3) {
			for (int i = 0; i < 3; ++i) {
				GLuint v = faces[j + i];
				const GLfloat* neighbor1 = &vertices[faces[j + (i + 1) % 3] * 3];
				const GLfloat* neighbor2 = &vertices[faces[j + (i + 2) % 3] * 3];

				new_positions[v * 3] += neighbor1[0] + neighbor2[0];
				new_positions[v * 3 + 1] += neighbor1[1] + neighbor2[1];
				new_positions[v * 3 + 2] += neighbor1[2] + neighbor2[2];
				counts[v] += 2;
			}
		}

		// Update vertex positions
		for (size_t i = 0; i < vertices.size() / 3; ++i) {
			new_positions[i * 3] /= counts[i];
			new_positions[i * 3 + 1] /= counts[i];
			new_positions[i * 3 + 2] /= counts[i];
			normalize(new_positions[i * 3], new_positions[i * 3 + 1], new_positions[i * 3 + 2]);
		}

		vertices = new_positions;
	}
}

size_t Model::Icosphere(int subdivisions) {
	float epsilon = 1e-6f;

	// vertex information for base icosahedron
	float phi = (1 + sqrtf(5)) / 2;
	std::vector<GLfloat> tempVer = {
		phi, 1.0f, 0.0f,
		phi, -1.0f, 0.0f,
		-phi, 1.0f, 0.0f,
		-phi, -1.0f, 0.0f,
		0.0f, phi, 1.0f,
		0.0f, phi, -1.0f,
		0.0f, -phi, 1.0f,
		0.0f, -phi, -1.0f,
		1.0f, 0.0f, phi,
		-1.0f, 0.0f, phi,
		1.0f, 0.0f, -phi,
		-1.0f, 0.0f, -phi
	};

	// rotate the icosahedron s.t. it can be neatly halved on its vertices on the plane x = 0
	float rotAngle = atan(phi);
	for (int i = 0; i < tempVer.size(); i += 3) {
		tempVer[i] = cosf(rotAngle) * tempVer[i] + sinf(rotAngle) * tempVer[i + 2];
		tempVer[i + 2] = -sin(rotAngle) * tempVer[i] + cosf(rotAngle) * tempVer[i + 2];
	}

	// initial face set as vertex indices
	std::vector<GLuint> tempInd = {
		2, 4, 5, 0, 5, 4, 4, 8, 0, 1, 0, 8, 10, 5, 0,
		2, 9, 4, 9, 2, 3, 11, 2, 5, 8, 9, 6, 4, 9, 8,
		11, 10, 7, 5, 10, 11, 3, 11, 7, 3, 2, 11, 6, 9, 3,
		1, 8, 6, 10, 0, 1, 7, 10, 1, 7, 6, 3, 6, 7, 1
	};

	std::unordered_map<uint64_t, GLuint> midpointCache;

	// Helper to compute a unique edge ID
	auto getEdgeID = [](GLuint a, GLuint b) -> uint64_t {
		return (static_cast<uint64_t>(std::min(a, b)) << 32) | std::max(a, b);
	};

	// Subdivide the faces
	for (int i = 0; i < subdivisions; ++i) {
		std::vector<GLuint> newFaces;

		for (int j = 0; j < tempInd.size(); j += 3) {
			GLuint v1 = tempInd[j];
			GLuint v2 = tempInd[j + 1];
			GLuint v3 = tempInd[j + 2];

			// Find or create midpoints
			uint64_t id1 = getEdgeID(v1, v2);
			uint64_t id2 = getEdgeID(v2, v3);
			uint64_t id3 = getEdgeID(v3, v1);

			GLfloat midX, midY, midZ;

			if (midpointCache.find(id1) == midpointCache.end()) {
				midpointCache[id1] = (GLuint)(tempVer.size() / 3);

				midX = tempVer[v1 * 3] + tempVer[v2 * 3];
				midY = tempVer[v1 * 3 + 1] + tempVer[v2 * 3 + 1];
				midZ = tempVer[v1 * 3 + 2] + tempVer[v2 * 3 + 2];
				normalize(midX, midY, midZ);

				tempVer.push_back(midX);
				tempVer.push_back(midY);
				tempVer.push_back(midZ);
			}

			if (midpointCache.find(id2) == midpointCache.end()) {
				midpointCache[id2] = (GLuint)(tempVer.size() / 3);

				midX = tempVer[v2 * 3] + tempVer[v3 * 3];
				midY = tempVer[v2 * 3 + 1] + tempVer[v3 * 3 + 1];
				midZ = tempVer[v2 * 3 + 2] + tempVer[v3 * 3 + 2];
				normalize(midX, midY, midZ);

				tempVer.push_back(midX);
				tempVer.push_back(midY);
				tempVer.push_back(midZ);
			}

			if (midpointCache.find(id3) == midpointCache.end()) {
				midpointCache[id3] = (GLuint)(tempVer.size() / 3);

				midX = tempVer[v3 * 3] + tempVer[v1 * 3];
				midY = tempVer[v3 * 3 + 1] + tempVer[v1 * 3 + 1];
				midZ = tempVer[v3 * 3 + 2] + tempVer[v1 * 3 + 2];
				normalize(midX, midY, midZ);

				tempVer.push_back(midX);
				tempVer.push_back(midY);
				tempVer.push_back(midZ);
			}

			// New vertices
			GLuint m1 = midpointCache[id1];
			GLuint m2 = midpointCache[id2];
			GLuint m3 = midpointCache[id3];

			std::vector<GLuint> indexSet = { v1, m1, m3, v2, m2, m1, v3, m3, m2, m1, m2, m3 };

			// Create new faces
			newFaces.insert(newFaces.end(), indexSet.begin(), indexSet.end());
		}

		tempInd = std::move(newFaces);
	}

	std::vector<GLuint> meridian;
	GLuint northPole, southPole;
	for (GLuint i = 0; i < tempVer.size(); i += 3) {
		if (abs(tempVer[i]) < epsilon && tempVer[i + 2] >= 0.0f) {
			meridian.push_back(i / 3);
			if (tempVer[i + 1] == 1.0f)
				northPole = i / 3;
			else if (tempVer[i + 1] == -1.0f)
				southPole = i / 3;
		}
	}

	relax_vertices(tempVer, tempInd);

	std::vector<GLfloat> tempTex;

	// Add the vertices, normals, and texture coordinates
	for (size_t i = 0; i < tempVer.size(); i += 3) {
		// map the vertex position to the unit sphere
		normalize(tempVer[i], tempVer[i + 1], tempVer[i + 2]);

		// assign longitude and latitude for texture mapping
		tempTex.push_back(0.5f - atan2(tempVer[i], -tempVer[i + 2]) / (2.0f * pi_f));
		tempTex.push_back(0.5f - asin(tempVer[i + 1]) / pi_f);
	}

	// normals all point directly away from the origin, and so are identical to the vertex information
	std::vector<GLfloat> tempNor = tempVer;
	
	for (GLuint meridianPoint : meridian) {
		// clone the meridian point, original for faces right of the seam
		size_t newPoint = duplicatePoint(tempVer, tempNor, tempTex, meridianPoint);
		if (tempTex[meridianPoint * 2] > 0.5f)
			tempTex[meridianPoint * 2] -= 1.0f;
		else
			tempTex[newPoint * 2] += 1.0f;

		for (int i = 0; i < tempInd.size(); i += 3) {
			int meridianInFace = -1;
			if (tempInd[i] == meridianPoint)
				meridianInFace = 0;
			else if (tempInd[i + 1] == meridianPoint)
				meridianInFace = 1;
			else if (tempInd[i + 2] == meridianPoint)
				meridianInFace = 2;

			if (meridianInFace != -1) {
				GLuint i1 = (meridianInFace + 1) % 3;
				GLuint i2 = (meridianInFace + 2) % 3;
				GLfloat u0 = tempTex[meridianPoint * 2];
				GLfloat u1 = tempTex[tempInd[i + i1] * 2];
				GLfloat u2 = tempTex[tempInd[i + i2] * 2];
				GLfloat avgU = (u0 + u1 + u2) / 3.0f;

				bool isLeft = std::find(meridian.begin(), meridian.end(), tempInd[i + i1]) == meridian.end() && u1 > 0.5f ||
					std::find(meridian.begin(), meridian.end(), tempInd[i + i2]) == meridian.end() && u2 > 0.5f;

				// triangle on left side of seam is assigned cloned point
				if (isLeft && meridianPoint != northPole && meridianPoint != southPole) {
					tempInd[i + meridianInFace] = (GLuint)newPoint;
				}
			}
		}
	}

	for (int i = 0; i < tempInd.size(); i += 3) {
		int poleInFace = -1;
		if (tempInd[i] == northPole || tempInd[i] == southPole)
			poleInFace = 0;
		else if (tempInd[i + 1] == northPole || tempInd[i + 1] == southPole)
			poleInFace = 1;
		else if (tempInd[i + 2] == northPole || tempInd[i + 2] == southPole)
			poleInFace = 2;

		if (poleInFace != -1) {
			GLuint i1 = (poleInFace + 1) % 3;
			GLuint i2 = (poleInFace + 2) % 3;

			GLuint newPoint = (GLuint)duplicatePoint(tempVer, tempNor, tempTex, tempInd[i + poleInFace]);

			tempTex[newPoint * 2] = (tempTex[tempInd[i + i1] * 2] + tempTex[tempInd[i + i2] * 2]) / 2;
			tempInd[i + poleInFace] = newPoint;
		}
	}

	// Add tangent and bitangent storage
	std::vector<GLfloat> tempTan(tempVer.size());
	std::vector<GLfloat> tempBitan(tempVer.size());

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

	modelLibrary.emplace_back(tempVer, tempInd, tempNor, tempTex, tempTan, tempBitan);
	return modelLibrary.size() - 1;
}

size_t Model::Ring(std::vector<GLfloat>& crossSection, size_t subdivisions, float fullness) {
	size_t n = crossSection.size() / 3;

	std::vector<GLfloat> vertices;
	vertices.reserve(n * subdivisions);
	std::vector<GLfloat> crossSectionCopy;
	crossSectionCopy.reserve(n);

	for (size_t angleFraction = 0; angleFraction < subdivisions; angleFraction++) {
		 crossSectionCopy = crossSection;
		 for (size_t i = 0; i < crossSection.size(); i += 3) {
			 float theta = pi_f * (2 * angleFraction) / subdivisions;
			 float r0 = (crossSectionCopy[i] - 0.5f) * fullness + 1;
			 crossSectionCopy[i + 1] *= fullness;

			 crossSectionCopy[i + 2] = r0 * sinf(theta);
			 crossSectionCopy[i] = r0 * cosf(theta);
		 }

		 vertices.insert(vertices.end(), crossSectionCopy.begin(), crossSectionCopy.end());
	}

	std::vector<GLuint> faces;
	faces.reserve(vertices.size());

	for (size_t k = 0; k < subdivisions; k++) {
		size_t l = k + 1 == subdivisions ? 0 : k + 1;
		for (size_t i = 0; i < n; i++) {
			size_t j = i + 1 == n ? 0 : i + 1;
			faces.push_back(GLuint((i | (size_t)0x01) % n + l * n));
			faces.push_back(GLuint(j + k * n));
			faces.push_back(GLuint(i + k * n));

			faces.push_back(GLuint(j + l * n));
			faces.push_back(GLuint((j & ~(size_t)0x01) % n + k * n));
			faces.push_back(GLuint(i + l * n));
		}
	}

	std::vector<GLfloat> normals, tex;

	toOBJ(vertices, faces);

	modelLibrary.emplace_back(vertices, faces, normals, tex);
	return modelLibrary.size() - 1;
}