#include "render.h"
#include <mutex>

glm::mat4 projection;
int screenSize = windowWidth;
float pipSize = 0.35f;

std::mutex physicsMutex;

size_t maxTrailLength = 2500;
size_t cameraFutureTrailLength = 2500;

GLuint trailVAO, trailVBO, trailAlphaBuf, quadVAO, quadVBO, pipFBO, pipTexture, pipDepthBuffer;
Shader shader, skyboxShader, trailShader, frameShader;

Camera pipCam(
	glm::dvec3(0, 1e6, 0),
	glm::dvec3(0, -1, 0),
	glm::dvec3(1, 0, 0));

void initShaders() {
	shader = initStandardShader();
	skyboxShader = initSkyboxShader();
	trailShader = initTrailShader();
	frameShader = initFrameOverlayShader();
}

void cleanGL() {
	glDeleteVertexArrays(1, &trailVAO);
	glDeleteBuffers(1, &trailVBO);
	glDeleteBuffers(1, &trailAlphaBuf);
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);

	glDeleteFramebuffers(1, &pipFBO);

	glDeleteProgram(shader.index);
	glDeleteProgram(skyboxShader.index);
	glDeleteProgram(trailShader.index);

	for (GravityBody body : bodies) {
		if (body.surface.texture)
			glDeleteTextures(1, &body.surface.texture);
		if (body.surface.normal)
			glDeleteTextures(1, &body.surface.normal);
	}

	glfwTerminate();
}

void setPV(Shader& shader, glm::mat4& P, glm::mat4& V) {
	glUniformMatrix4fv(shader.P, 1, GL_FALSE, &P[0][0]);
	glUniformMatrix4fv(shader.V, 1, GL_FALSE, &V[0][0]);
}

// update projection matrix based on window size
void updateProjectionMatrix(GLFWwindow* window) {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	if (height == 0) height = 1; // prevent division by zero

	float aspect = (float)width / (float)height;
	projection = glm::perspective(FOV * height / screenSize, aspect, 1e0f, 1e9f);
}

void updateTrails(double time, camera_mode mode) {
	for (GravityBody body : bodies) {
		auto trail = body.trail;

		if (trail && body.mass > 1.0 || mode == GRAV_CAM) {
			size_t parentIndex = body.parentIndex;

			// trail relative to parent body
			if (parentIndex != -1) {
				if (trail->size() > 1) {
					bool doLoop = true;

					// remove any trail points behind the body's position
					while (doLoop) {
						glm::dvec3 a = trail->front();
						glm::dvec3 vel = body.velocity - bodies[parentIndex].velocity;
						glm::dvec3 b = trail->back();

						glm::dvec3 n = glm::cross(b, body.position - bodies[parentIndex].position);

						// get angle between the last point added to the trail and the start of the trail
						double angle = acos(glm::dot(a, b) / (glm::length(a) * glm::length(b)));
						if (glm::dot(n, glm::cross(a, b)) < 0)
							angle *= -1;

						// get angle between the body's current position and the start of the trail
						b = body.position - bodies[parentIndex].position;
						double angle2 = acos(glm::dot(a, b) / (glm::length(a) * glm::length(b)));
						if (glm::dot(n, glm::cross(a, b)) < 0)
							angle2 *= -1;

						if (angle < 0 && angle2 >= 0 || std::isnan(angle) || std::isnan(angle2))
							trail->pop_front();
						else
							doLoop = false;
					}
				}

				trail->push_back(glm::vec3(body.position - bodies[parentIndex].position));

			}

			// trail relative to world space
			else {
				trail->push_back(body.position);
				while (trail->size() > maxTrailLength) {
					trail->pop_front();
				}
			}
		}
	}
}

void updateFreeCam(double deltaTime) {
	camera.position += camera.velocity * deltaTime;
	camera.velocity = glm::dvec3(0);
}

void updateLockCam(double deltaTime) {
	if (lockIndex != -1) {
		GravityBody* body = &bodies[lockIndex];
		if (body->parentIndex != -1) {
			camera.direction = glm::dvec3(0, -1, 0);
			camera.up = glm::normalize(body->position - bodies[body->parentIndex].position);
			camera.right = glm::cross(camera.direction, camera.up);
		}
		camera.position = body->position - lockDistanceFactor * body->radius * camera.direction;
	}
}

void updateGravCam(double deltaTime) {
	if (hasPhysics) {
		// bind camera to gravity-bound object
		camera.position = bodies[bodies.size() - 1].position;
		bodies[bodies.size() - 1].velocity += camera.velocity;
		camera.velocity = glm::dvec3(0);

		// project camera object's future movement in the system
		std::vector<GravityBody> copy = bodies;
		for (int i = 0; i < cameraFutureTrailLength; i++) {
			updateBodies(deltaTime, copy);
			bodies[bodies.size() - 1].trail->push_back(copy[copy.size() - 1].position);
		}
	}
}

void updateCamera(double deltaTime) {
	switch (camera.mode) {
	case FREE_CAM:
		updateFreeCam(deltaTime);
		break;
	case LOCK_CAM:
		updateLockCam(deltaTime);
		break;
	case GRAV_CAM:
		updateGravCam(deltaTime);
		break;
	}
}

void drawQuad() {
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glDrawArrays(GL_TRIANGLES, 0, 6); // 6 vertices for a quad
	glBindVertexArray(0);
}

void render(Camera& camera) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Activate the shader program
	glUseProgram(shader.index);
	glm::vec3 lightPos = bodies[0].position;
	glm::vec3 lightColor = glm::vec3(1.0f);
	glUniform3fv(shader.uniforms[LIGHT_POS], 1, &lightPos[0]);
	glUniform3fv(shader.uniforms[LIGHT_COLOR], 1, &lightColor[0]);

	// set camera
	glm::mat4 view = camera.viewMatrix();
	setPV(shader, projection, view);

	Camera copy = camera;
	for (Entity body : bodies) {
		copy.position = camera.position - body.position;
		glm::mat4 relativeView = copy.viewMatrix();
		glUniform3fv(shader.uniforms[LIGHT_POS], 1, &(lightPos - glm::vec3(body.position))[0]);
		setPV(shader, projection, relativeView);
		glm::dvec3 bodyPos = body.position;
		body.position = glm::dvec3(0.0);
		body.draw(shader, MODE_TEX);
		body.position = bodyPos;
	}

	// start trails

	glUseProgram(trailShader.index);
	setPV(trailShader, projection, view);
	std::vector<glm::vec3> trailVertices;
	std::vector<float> trailAlphas;
	for (GravityBody body : bodies) {
		if (body.trail) {
			trailVertices.insert(trailVertices.end(), body.trail->begin(), body.trail->end());
			for (size_t j = 0; j < body.trail->size(); ++j) {
				trailAlphas.push_back(static_cast<float>(j) / body.trail->size());
			}
		}
	}

	glBindVertexArray(trailVAO);
	glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
	glBufferData(GL_ARRAY_BUFFER, trailVertices.size() * sizeof(glm::vec3), trailVertices.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0); // Assuming location 0 for positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

	glBindBuffer(GL_ARRAY_BUFFER, trailAlphaBuf);
	glBufferData(GL_ARRAY_BUFFER, trailAlphas.size() * sizeof(float), trailAlphas.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(1); // Assuming location 1 for alphas
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), 0);

	GLint offset = 0;
	glm::vec3 root(0.0);
	for (GravityBody body : bodies) {
		if (body.trail) {
			glUniform3fv(trailShader.uniforms[OBJ_COLOR], 1, &body.trailColor[0]);
			if (body.parentIndex != -1)
				root = glm::vec3(bodies[body.parentIndex].position);
			else
				root = glm::vec3(0.0);
			glUniform3fv(trailShader.uniforms[OBJ_POS], 1, &root[0]);
			glDrawArrays(GL_LINE_STRIP, offset, (GLsizei)body.trail->size());
			offset += (GLint)body.trail->size();
		}
	}

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glBindVertexArray(0); // Unbind VAO

	// end trails

	glDepthFunc(GL_LEQUAL);
	glUseProgram(skyboxShader.index);

	// set camera
	view = glm::mat4(glm::mat3(view));
	setPV(skyboxShader, projection, view);

	Entity::skybox.draw(skyboxShader, MODE_CUBEMAP);

	glDepthFunc(GL_LESS);
}

void renderPIP(GLFWwindow* window) {
	glBindFramebuffer(GL_FRAMEBUFFER, pipFBO);
	GLsizei pipWidth = GLsizei(windowWidth * pipSize);
	GLsizei pipHeight = GLsizei(windowHeight * pipSize);
	glViewport(0, 0, pipWidth, pipHeight);
	// update pip size
	glBindTexture(GL_TEXTURE_2D, pipTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pipWidth, pipHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pipTexture, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, pipDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, pipWidth, pipHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pipDepthBuffer);

	// Render the scene for the PIP view
	render(pipCam);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
	updateProjectionMatrix(window);

	// Step 3: Render the PIP texture to a quad in the main screen
	glUseProgram(frameShader.index);

	// Define the transformation for the PIP view on the screen (e.g., top-right corner)
	glm::mat4 pipTransform = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f - pipSize, -1.0f + pipSize, 0.0f));  // Position in the bottom right corner
	pipTransform = glm::scale(pipTransform, glm::vec3(pipSize, pipSize, 1.0f));  // Scale to fit

	// Set the transformation for the PIP quad
	glUniformMatrix4fv(frameShader.M, 1, GL_FALSE, &pipTransform[0][0]);

	// Bind the PIP texture and render the quad
	glBindTexture(GL_TEXTURE_2D, pipTexture);
	drawQuad();  // Assume a quad is already defined for rendering
}

void renderLoop(GLFWwindow* window) {
	double lastFrameTime = glfwGetTime();
	double currentTime = glfwGetTime();

	while (!glfwWindowShouldClose(window)) {
		if (currentTime - lastFrameTime > MIN_FRAME_TIME) {
			// capture physics results when they are ready
			std::unique_lock<std::mutex> lock(physicsMutex);
			physicsCV.wait(lock, [] { return physicsUpdated; });
			physicsUpdated = false;
			lastPhysicsFrames = physicsFrames;
			lock.unlock();


			updateCamera(currentTime - lastFrameTime);


			if (hasPhysics)
				updateTrails(currentTime - lastFrameTime, camera.mode);
			render(camera);

			if (hasPhysics && camera.mode == GRAV_CAM) {
				// clean up future trail
				for (int i = 0; i < cameraFutureTrailLength; i++)
					bodies[bodies.size() - 1].trail->pop_back();
			}

			renderPIP(window);

			lastFrameTime = currentTime;

			glfwSwapBuffers(window);
			glfwPollEvents();
		}

		currentTime = glfwGetTime();
	}
}
