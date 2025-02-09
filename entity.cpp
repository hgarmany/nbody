#include "entity.h"
#include <glm/gtc/quaternion.hpp>

Entity Entity::skybox;

// rotates on local axes in pitch, yaw, roll order
glm::dquat Entity::getRotationQuat() {
	glm::dquat pitchQuat = glm::angleAxis(orientation.x, glm::dvec3(1.0, 0.0, 0.0));
	glm::dvec3 direction = pitchQuat * glm::dvec3(0.0, 0.0, -1.0);
	glm::dvec3 up = pitchQuat * glm::dvec3(0.0, 1.0, 0.0);

	glm::dquat yawQuat = glm::angleAxis(orientation.y, up);
	direction = yawQuat * direction;
	glm::dvec3 right = yawQuat * glm::dvec3(1.0, 0.0, 0.0);

	glm::dquat rollQuat = glm::angleAxis(orientation.z, direction);
	up = rollQuat * up;
	right = rollQuat * right;

	return rollQuat * yawQuat * pitchQuat;
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

	glDrawElements(GL_TRIANGLES, Model::modelLibrary[modelIndex].numFaces, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}