#include "entity.h"
#include <glm/gtc/quaternion.hpp>

glm::dquat Entity::getRotationQuat() {
	glm::float64 sinYaw2 = sin(orientation.y * 0.5f);
	glm::float64 sinPitch2 = sin(orientation.x * 0.5f);
	glm::float64 sinRoll2 = sin(orientation.z * 0.5f);
	glm::float64 cosYaw2 = cos(orientation.y * 0.5f);
	glm::float64 cosPitch2 = cos(orientation.x * 0.5f);
	glm::float64 cosRoll2 = cos(orientation.z * 0.5f);

	glm::float64 w = cosYaw2 * cosPitch2 * cosRoll2 + sinYaw2 * sinPitch2 * sinRoll2;
	glm::float64 x = cosYaw2 * sinPitch2 * cosRoll2 + sinYaw2 * cosPitch2 * sinRoll2;
	glm::float64 y = sinYaw2 * cosPitch2 * cosRoll2 - cosYaw2 * sinPitch2 * sinRoll2;
	glm::float64 z = cosYaw2 * cosPitch2 * sinRoll2 - sinYaw2 * sinPitch2 * cosRoll2;

	return glm::dquat(w, x, y, z);
}

glm::dmat4 Entity::updateMatrix() {
	modelMatrix = glm::translate(glm::dmat4(1.0), position);
	modelMatrix *= glm::mat4_cast(getRotationQuat());
	modelMatrix = glm::scale(modelMatrix, scale);

	return modelMatrix;
}

void Entity::draw(Shader shader, uint8_t mode) {
	glBindVertexArray(Model::modelLibrary[modelIndex].VAO);

	if (shader.uniforms.contains(OBJ_COLOR))
		glUniform3fv(shader.uniforms[OBJ_COLOR], 1, &surface.color[0]);
	if (shader.uniforms.contains(OBJ_MAT))
		glUniform4fv(shader.uniforms[OBJ_MAT], 1, &surface.material[0]);


	// Handle displacement map for the vertex shader
	if (surface.normal != -1) {
		glActiveTexture(GL_TEXTURE1); // Use a different texture unit
		glBindTexture(GL_TEXTURE_2D, surface.normal);
		glUniform1i(shader.uniforms[NORMAL_MAP], 1); // Tell shader to use texture unit 1
		glUniform1i(shader.uniforms[NORM_BOOL], 1);
	}
	else if (mode == MODE_TEX) {
		glUniform1i(shader.uniforms[NORM_BOOL], 0);
	}

	if (surface.texture != -1) {
		glActiveTexture(GL_TEXTURE0);
		if (mode == MODE_TEX) {
			glBindTexture(GL_TEXTURE_2D, surface.texture);
			glUniform1i(shader.uniforms[TEX_MAP], 0);
			glUniform1i(shader.uniforms[TEX_BOOL], 1);
		}
		else if (mode == MODE_CUBEMAP) {
			glBindTexture(GL_TEXTURE_CUBE_MAP, surface.texture);
		}
	}
	else {
		glBindTexture(GL_TEXTURE_2D, 0);
		glUniform1i(shader.uniforms[TEX_BOOL], 0);
	}

	if (mode != MODE_CUBEMAP) {
		glm::mat4 M = updateMatrix();
		glUniformMatrix4fv(shader.M, 1, GL_FALSE, &M[0][0]);
	}

	glDrawElements(GL_TRIANGLES, (GLsizei)Model::modelLibrary[modelIndex].indexLength, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}