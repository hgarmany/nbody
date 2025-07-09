#include "entity.h"

std::shared_ptr<Entity> Entity::skybox;

std::vector<std::shared_ptr<Entity>> entities;

glm::dmat4 Entity::updateMatrix(bool doScale) {
	modelMatrix = glm::dmat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, position);
	modelMatrix *= glm::dmat4(rotQuat);
	if (doScale)
		modelMatrix = glm::scale(modelMatrix, scale);

	return modelMatrix;
}

void Entity::draw(Shader& shader, uint8_t mode, glm::dmat4 rootMatrix) {
	if (modelIndex == -1)
		return; // cannot draw an object with no model

	glBindVertexArray(Model::modelLibrary[modelIndex].VAO);
	
	if (shader.uniforms.contains(OBJ_COLOR))
		glUniform3fv(shader.uniforms[OBJ_COLOR], 1, &surface.color[0]);
	if (shader.uniforms.contains(OBJ_MAT))
		glUniform4fv(shader.uniforms[OBJ_MAT], 1, &surface.material[0]);

	// Handle displacement map for the vertex shader
	if (surface.getNormalMap()) {
		glActiveTexture(GL_TEXTURE1); // Use a different texture unit
		glBindTexture(GL_TEXTURE_2D, surface.getNormalMap());
		glUniform1i(shader.uniforms[NORMAL_MAP], 1); // Tell shader to use texture unit 1
		glUniform1i(shader.uniforms[NORM_BOOL], 1);
	}
	else if (shader.uniforms.count(NORM_BOOL))
		glUniform1i(shader.uniforms[NORM_BOOL], 0);

	GLuint texID = surface.getTexture();

	if (texID) {
		glActiveTexture(GL_TEXTURE0);
		if (mode == MODE_TEX) {
			glBindTexture(GL_TEXTURE_2D, texID);
			glUniform1i(shader.uniforms[TEX_MAP], 0);
			glUniform1i(shader.uniforms[TEX_BOOL], 1);
		}
		else if (mode == MODE_CUBEMAP) {
			glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
		}
	}
	else {
		glBindTexture(GL_TEXTURE_2D, 0);
		glUniform1i(shader.uniforms[TEX_BOOL], 0);
	}

	if (mode != MODE_CUBEMAP) {
		glm::dvec3 temp = position;
		position = glm::dvec3(0.0);
		glm::mat4 M = updateMatrix() * rootMatrix;
		glUniformMatrix4fv(shader.M, 1, GL_FALSE, &M[0][0]);
		position = temp;
	}
	
	glDrawElements(GL_TRIANGLES, Model::modelLibrary[modelIndex].numFaces, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}