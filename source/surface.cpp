#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "surface.h"

GLuint Surface::importTexture(const char* path, bool useInterpolation) {
	GLuint texture = 0;

	int width, height, numChannels;
	unsigned char* data = stbi_load(path, &width, &height, &numChannels, 0);

	if (data) {
		GLint internalFormat = GL_RGB, format = GL_RGB;

		switch (numChannels) {
		case 1:
			internalFormat = GL_R8;
			format = GL_RED;
			break;
		case 3:
			break;
		case 4:
			internalFormat = format = GL_RGBA;
			break;
		default:
			fprintf(stderr, "%s:\tunsupported image format (%u channels)\n", path, numChannels);
		}

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);

		if (useInterpolation)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		else
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else {
		fprintf(stderr, "%s:\tcould not find texture\n", path);
	}

	stbi_image_free(data);
	return texture;
}

Surface::Surface(const char* path, glm::vec4 material, glm::vec3 color) {
	this->material = material;
	this->color = color;
	texture = importTexture(path);
	normal = 0;
}

Surface Surface::CubeMap(std::vector<std::string> faces) {
	Surface out;
	glGenTextures(1, &out.texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, out.texture);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++) {
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else {
			fprintf(stderr, "Cubemap texture failed to load at path: %s\n", faces[i].c_str());
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return out;
}

void Surface::setNormal(const char* path) {
	normal = importTexture(path);
}

GLuint Surface::getTexture() {
	return texture;
}

GLuint Surface::getNormalMap() {
	return normal;
}
