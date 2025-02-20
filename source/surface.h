#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <future>

#include <iostream>

#include "stb_image.h"

GLuint importTexture(const char* path, bool useInterpolation = false);

class Surface {
private:
	std::shared_ptr<std::future<GLuint>> textureFuture, normalFuture;
	GLuint texture, normal;
public:
	glm::vec4 material;
	glm::vec3 color;

	Surface(glm::vec4 material = glm::vec4(0.0f), glm::vec3 color = glm::vec3(1.0f)) {
		this->material = material;
		this->color = color;
		texture = 0;
		normal = 0;
	}

	Surface(const char* path, glm::vec4 material = glm::vec4(0.0f), glm::vec3 color = glm::vec3(1.0f));

	static Surface CubeMap(std::vector<std::string> faces);

	void setNormal(const char* path);
	GLuint getTexture();
	GLuint getNormalMap();
};