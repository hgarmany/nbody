#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "stb_image.h"

class Surface {
public:
	glm::vec4 material;
	glm::vec3 color;
	GLuint texture;

	Surface(glm::vec4 material = glm::vec4(0.0f), glm::vec3 color = glm::vec3(0.0f)) {
		this->material = material;
		this->color = color;
		texture = -1;
	}

	Surface(const char* path, glm::vec4 material, glm::vec3 color);
};