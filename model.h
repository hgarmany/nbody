#pragma once

#include <GL/glew.h>
#include <vector>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "util.h"

class Model {
private:
	static std::vector<GLfloat> vertexLibrary;
	static std::vector<GLuint> indexLibrary;
	static std::vector<GLfloat> normalLibrary;
	static std::vector<GLfloat> texLibrary;
	static size_t ModelFromImportedVectors(std::vector<GLfloat>& verts, std::vector<GLuint>& indices, std::vector<GLfloat>& normals, std::vector<GLfloat>& tex);

public:
	static std::vector<Model> modelLibrary;

	size_t vertsStart, vertsLength, indexStart, indexLength;
	GLuint VAO, VBO, EBO, VNor, TexBuf;

	Model(
		size_t vertsStart, size_t vertsLength,
		size_t indexStart, size_t indexLength,
		size_t normalStart, size_t normalLength,
		size_t texStart, size_t texLength
	) {

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &VNor);
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

		glBindBuffer(GL_ARRAY_BUFFER, VNor);
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

	size_t generateNormals();

	static size_t Cube();
	static size_t Sphere();
	static size_t Icosphere(int subdivisions = 0);


	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;
	Model(Model&& other) {
		vertsStart = other.vertsStart;
		vertsLength = other.vertsLength;
		indexStart = other.indexStart;
		indexLength = other.indexLength;

		VAO = other.VAO;
		VBO = other.VBO;
		EBO = other.EBO;
		VNor = other.VNor;
		TexBuf = other.TexBuf;

		other.VAO = 0;
		other.VBO = 0;
		other.VNor = 0;
		other.TexBuf = 0;
	}

	~Model() {
		if (VBO) { glDeleteBuffers(1, &VBO); }
		if (EBO) { glDeleteBuffers(1, &EBO); }
		if (VNor) { glDeleteBuffers(1, &VNor); }
		if (TexBuf) { glDeleteBuffers(1, &TexBuf); }
		if (VAO) { glDeleteVertexArrays(1, &VAO); }
	}
};