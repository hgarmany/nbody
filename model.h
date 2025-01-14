#pragma once

#include <GL/glew.h>
#include <vector>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "util.h"

class Model {
public:
	std::vector<GLfloat> vertices;
	std::vector<GLfloat> normals;
	std::vector<GLfloat> texCoords;
	std::vector<GLuint> indices;
	GLuint VAO, VBO, EBO, VNor, VTex;

	Model(
		std::vector<GLfloat> vertices, 
		std::vector<GLuint> indices, 
		std::vector<GLfloat> normals = {}, 
		std::vector<GLfloat> texCoords = {}
	) {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &VNor);
		glGenBuffers(1, &VTex);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		this->vertices = vertices;
		this->indices = indices;

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(GLfloat), this->vertices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0); // Position

		if (normals.size() == 0)
			generateNormals();
		else
			this->normals = normals;

		glBindBuffer(GL_ARRAY_BUFFER, VNor);
		glBufferData(GL_ARRAY_BUFFER, this->normals.size() * sizeof(GLfloat), this->normals.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0); // Normals

		this->texCoords = texCoords;

		glBindBuffer(GL_ARRAY_BUFFER, VTex);
		glBufferData(GL_ARRAY_BUFFER, this->texCoords.size() * sizeof(GLfloat), this->texCoords.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0); // Texture coordinates

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), this->indices.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	void generateNormals();

	static Model Cube();
	static Model Sphere();
	static Model Icosphere(int subdivisions = 0);

	~Model() {
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
		glDeleteBuffers(1, &VNor);
		glDeleteBuffers(1, &VTex);
		glDeleteVertexArrays(1, &VAO);
	}
};