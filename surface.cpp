#define STB_IMAGE_IMPLEMENTATION
#include "surface.h"

Surface::Surface(const char* path, glm::vec4 material, glm::vec3 color) {
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

	if (data) {
		GLuint colorType = nrChannels == 4 ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	else {
		std::cerr << "Texture failed to load at path: " << path << "\n";
	}

	stbi_image_free(data);

	this->material = material;
	this->color = color;
}