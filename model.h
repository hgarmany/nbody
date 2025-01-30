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
	static size_t ModelFromImportedVectors(std::vector<GLfloat>& verts, std::vector<GLuint>& indices, std::vector<GLfloat>& normals, std::vector<GLfloat>& tex, std::vector<GLfloat>& tan, std::vector<GLfloat>& bitan);
	static size_t ModelFromImportedVectors(std::vector<GLfloat>& verts, std::vector<GLuint>& indices, std::vector<GLfloat>& normals, std::vector<GLfloat>& tex);

public:
	static std::vector<Model> modelLibrary;

	size_t vertsStart, vertsLength, indexStart, indexLength;
	GLuint VAO, VBO, EBO, NorBuf, TexBuf, TanBuf, BitanBuf;

	Model(
		size_t vertsStart, size_t vertsLength,
		size_t indexStart, size_t indexLength,
		size_t normalStart, size_t normalLength,
		size_t texStart, size_t texLength,
		std::vector<GLfloat>& tan, std::vector<GLfloat>& bitan
	);

	Model(
		size_t vertsStart, size_t vertsLength,
		size_t indexStart, size_t indexLength,
		size_t normalStart, size_t normalLength,
		size_t texStart, size_t texLength
	);

	size_t generateNormals();

	static size_t Cube();
	static size_t Sphere();
	static size_t Icosphere(int subdivisions = 0);


	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;
	Model(Model&& other) noexcept {
		vertsStart = other.vertsStart;
		vertsLength = other.vertsLength;
		indexStart = other.indexStart;
		indexLength = other.indexLength;

		VAO = other.VAO;
		VBO = other.VBO;
		EBO = other.EBO;
		NorBuf = other.NorBuf;
		TexBuf = other.TexBuf;

		other.VAO = 0;
	}

	~Model() {
		if (VAO) {
			glDeleteBuffers(1, &VBO);
			glDeleteBuffers(1, &EBO);
			glDeleteBuffers(1, &NorBuf);
			glDeleteBuffers(1, &TexBuf);
			glDeleteVertexArrays(1, &VAO);
		}
	}
};