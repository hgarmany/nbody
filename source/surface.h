#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

class Surface {
private:
	GLuint texture, normal;
public:
	glm::vec4 material;
	glm::vec3 color;

	Surface(glm::vec4 material = glm::vec4(0.0f), glm::vec3 color = glm::vec3(1.0f)) :
		material(material), color(color), texture(0), normal(0) {};

	Surface(const char* path, glm::vec4 material = glm::vec4(0.0f), glm::vec3 color = glm::vec3(1.0f));
	
	static GLuint importTexture(const char* path, bool useInterpolation = false);
	static Surface CubeMap(std::vector<std::string> faces);

	void setNormal(const char* path);
	GLuint getTexture();
	GLuint getNormalMap();
};