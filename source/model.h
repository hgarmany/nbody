#pragma once

#include <GL/glew.h>
#include <vector>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include "util.h"

class Model {
public:
	static std::vector<Model> modelLibrary;

	GLuint VAO = 0, VBO = 0, EBO = 0, NorBuf = 0, TexBuf = 0, TanBuf = 0, BitanBuf= 0;
	GLsizei numFaces = 0;

	Model(
		std::vector<GLfloat>& verts, std::vector<GLuint>& indices,
		std::vector<GLfloat>& normals, std::vector<GLfloat>& tex,
		std::vector<GLfloat>& tan, std::vector<GLfloat>& bitan
	);

	Model(
		std::vector<GLfloat>& verts, std::vector<GLuint>& indices,
		std::vector<GLfloat>& normals, std::vector<GLfloat>& tex
	);

	std::vector<GLfloat> generateNormals(std::vector<GLfloat>& verts, std::vector<GLuint>& indices);

	static size_t Cube();
	static size_t Sphere();
	static size_t Icosphere(int subdivisions = 0);
	static size_t Ring(std::vector<GLfloat>& crossSection, size_t subdivisions, float fullness);
	static size_t Square();


	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;
	Model(Model&& other) noexcept {
		VAO = other.VAO;
		VBO = other.VBO;
		EBO = other.EBO;
		NorBuf = other.NorBuf;
		TexBuf = other.TexBuf;
		TanBuf = other.TanBuf;
		BitanBuf = other.BitanBuf;
		numFaces = other.numFaces;

		other.VAO = 0;
	}

	~Model() {
		if (VAO) {
			glDeleteBuffers(1, &VBO);
			glDeleteBuffers(1, &EBO);
			glDeleteBuffers(1, &NorBuf);
			glDeleteBuffers(1, &TexBuf);
			glDeleteBuffers(1, &TanBuf);
			glDeleteBuffers(1, &BitanBuf);
			glDeleteVertexArrays(1, &VAO);
		}
	}
};