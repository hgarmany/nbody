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

void Entity::draw(Shader shader) {
	glUniform3fv(shader.objectColor, 1, &surface.color[0]);
	glUniform4fv(shader.objectMat, 1, &surface.material[0]);

	if (surface.texture != -1) {
		glBindTexture(GL_TEXTURE_2D, surface.texture);
		glUniform1i(shader.texBool, 1);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, 0);
		glUniform1i(shader.texBool, 0);
	}

	glm::mat4 M = updateMatrix();
	glUniformMatrix4fv(shader.M, 1, GL_FALSE, &M[0][0]);
	glBindVertexArray(model->VAO);
	glDrawElements(GL_TRIANGLES, (GLsizei)model->indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}