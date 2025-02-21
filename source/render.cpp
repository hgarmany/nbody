#include "render.h"
#include <mutex>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

GLFWwindow* window;
glm::mat4 projection;
int screenSize = windowWidth;
float pipSize = 0.35f;

std::mutex physicsMutex;

size_t maxTrailLength = 2500;
size_t cameraFutureTrailLength = 2500;
GLsizei starCapacity = 20;
GLsizei trailCapacity = 0;

GLuint trailVAO, trailVBO, trailAlphaBuf, quadVAO, quadVBO, pipFBO, pipTexture, pipDepthBuffer, instanceVBO, starTex;
Shader shader, skyboxShader, trailShader, frameShader, spriteShader;

std::vector<glm::vec3> trailVertices;
std::vector<float> trailAlphas;

void initCamera() {
	glm::dvec3 direction = glm::normalize(bodies[bodies.size() - 1].position - bodies[0].position);
	glm::dvec3 right(1.0, 0.0, 0.0);
	glm::dvec3 up = glm::cross(right, direction);

	camera = Camera(
		bodies[bodies.size() - 1].position,
		direction, up
	);
	camera.mode = LOCK_CAM;
	camera.lockIndex = 0;

	// obtain initial perspective information from relationship between window and screen
	GLFWmonitor* screen = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(screen);
	screenSize = mode->height;
	projection = glm::perspective(FOV * windowHeight / screenSize, (float)windowWidth / windowHeight, 1e-1f, 1e9f);
}

// setup for the simple display quad
void initQuad() {
	GLfloat quadVertices[] = {
		// Positions       // Texture Coords
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // Bottom-left
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  // Bottom-right
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // Top-right
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // Top-right
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // Top-left
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f   // Bottom-left
	};

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);

	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	// load vertex data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// load uv mapping
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
}

// prepares an FBO for capturing renders
void initPIP() {
	glGenFramebuffers(1, &pipFBO);
	glGenTextures(1, &pipTexture);
	glGenRenderbuffers(1, &pipDepthBuffer);

	GLsizei pipWidth = GLsizei(windowWidth * pipSize);
	GLsizei pipHeight = GLsizei(windowHeight * pipSize);

	// texture space for holding render data
	glBindTexture(GL_TEXTURE_2D, pipTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pipWidth, pipHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// attach texture buffer
	glBindFramebuffer(GL_FRAMEBUFFER, pipFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pipTexture, 0);

	// attach depth buffer
	glBindRenderbuffer(GL_RENDERBUFFER, pipDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, pipWidth, pipHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pipDepthBuffer);
}

void initShaders() {
	shader = initStandardShader();
	skyboxShader = initSkyboxShader();
	trailShader = initTrailShader();
	frameShader = initFrameOverlayShader();
	spriteShader = initSpriteShader();
}

void initTrails() {
	glGenVertexArrays(1, &trailVAO);
	glGenBuffers(1, &trailVBO);
	glGenBuffers(1, &trailAlphaBuf);
}

void initStarBuffer() {
	starTex = Surface::importTexture("../../assets/star.png", true);

	glBindVertexArray(quadVAO); // Use existing quadVAO

	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, starCapacity * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

	// Setup instance attribute (location = 2 for position)
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glVertexAttribDivisor(2, 1); // Update per instance
}

// grab star positions based on bodies in the system
GLsizei updateStarPositions() {
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

	std::vector<glm::vec3> positions;
	positions.reserve(frameBodies.size());
	for (GravityBody& body : frameBodies) {
		if (body.mass > 1e5f)
			positions.push_back(body.position - camera.position);
	}
	
	GLsizei size = (GLsizei)positions.size();
	
	// reallocate buffer if the system requires more stars than it can hold
	if (size > starCapacity) {
		glBufferData(GL_ARRAY_BUFFER, 2 * size * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
		starCapacity = 2 * size;
	}
	
	glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(glm::vec3), positions.data());  // Update positions

	return size;
}

void cleanup() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glDeleteVertexArrays(1, &trailVAO);
	glDeleteBuffers(1, &trailVBO);
	glDeleteBuffers(1, &trailAlphaBuf);
	glDeleteTextures(1, &starTex);
	glDeleteBuffers(1, &instanceVBO);
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);

	glDeleteTextures(1, &pipTexture);
	glDeleteRenderbuffers(1, &pipDepthBuffer);
	glDeleteFramebuffers(1, &pipFBO);

	for (GravityBody& body : bodies) {
		GLuint tex = body.surface.getTexture();
		GLuint nor = body.surface.getNormalMap();
		if (tex)
			glDeleteTextures(1, &tex);
		if (nor)
			glDeleteTextures(1, &nor);
	}

	glfwTerminate();
}

void setPV(Shader& shader, glm::mat4& P, glm::mat4& V) {
	glUniformMatrix4fv(shader.P, 1, GL_FALSE, &P[0][0]);
	glUniformMatrix4fv(shader.V, 1, GL_FALSE, &V[0][0]);
}

// update projection matrix based on window size
void updateProjectionMatrix() {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	if (height == 0) height = 1; // prevent division by zero

	float aspect = (float)width / (float)height;
	projection = glm::perspective(FOV * height / screenSize, aspect, 1e-1f, 1e9f);
}

glm::dmat4 relativeRotationalMatrix(GravityBody* subject, GravityBody* reference, bool detranslate = false) {
	// develops a rotational matrix to transform subject coordinates to a reference frame in which the reference body and its parent both lie along the x axis
	glm::dmat4 rotate(1.0);
	if (reference != &frameBodies[subject->parentIndex]) {
		size_t aParentIndex = reference->parentIndex;
		glm::dvec3 a = glm::normalize(reference->position - frameBodies[aParentIndex].position);
		glm::dvec3 b(1.0, 0.0, 0.0);
		glm::dvec3 n = glm::cross(a, reference->velocity);

		double angle = acos(glm::dot(a, b));
		if (glm::dot(n, glm::cross(a, b)) < 0)
			angle *= -1;

		// shaders will detranslate spatial adjustments
		if (detranslate)
			rotate = glm::rotate(rotate, -angle, n);
		else
			rotate = glm::rotate(rotate, angle, n);
	}
	return rotate;
}

bool testObjectVisibility(Entity& entity, Camera& camera) {
	double longestPossibleAxisLength = glm::length(entity.scale);
	double angularSize = 2.0 * atan2(longestPossibleAxisLength, glm::distance(camera.position, entity.position));
	if (windowHeight * angularSize / FOV < 0.5) {
		return false;
	}
	return true;
}

void updateTrails() {
	for (GravityBody body : frameBodies) {
		Trail* trail = body.trail;
		if (trail && (body.mass > 1.0 || camera.mode == GRAV_CAM)) {
			size_t parentIndex = body.trail->parentIndex;

			// trail relative to parent body
			if (parentIndex != -1) {
				if (trail->size() > 1) {
					bool doLoop = true;

					// remove any trail points behind the body's position
					while (doLoop &&
						parentIndex == body.parentIndex) {
						glm::dvec3 a = glm::normalize(trail->front());
						glm::dvec3 b = glm::normalize(trail->back());

						glm::dvec3 n = glm::cross(b, body.position - frameBodies[parentIndex].position);

						// get angle between the last point added to the trail and the start of the trail
						double angle = acos(glm::dot(a, b));
						if (glm::dot(n, glm::cross(a, b)) < 0)
							angle *= -1;

						// get angle between the body's current position and the start of the trail
						b = glm::normalize(body.position - frameBodies[parentIndex].position);
						double angle2 = acos(glm::dot(a, b));
						if (glm::dot(n, glm::cross(a, b)) < 0)
							angle2 *= -1;

						if (angle < 0 && angle2 >= 0 || std::isnan(angle) || std::isnan(angle2))
							trail->pop();
						else
							doLoop = false;
					}
				}
				
				glm::dvec3 relativePosition = 
					glm::dmat3(relativeRotationalMatrix(&body, &frameBodies[parentIndex])) *
					glm::dvec3(body.position - frameBodies[body.parentIndex].position);
					//glm::dvec3(body.position);

				trail->push(relativePosition);
			}
			// trail relative to world space
			else {
				trail->push(body.position);
				while (trail->size() > maxTrailLength) {
					trail->pop();
				}
			}
		}
	}
}

void updateFreeCam(Camera& eye, double deltaTime) {
	if (&eye == &camera)
		flyCam(window, deltaTime);
}

void updateLockCam(Camera& eye, double deltaTime) {
	rollCamera(window, deltaTime);

	if (eye.lockIndex >= frameBodies.size() && eye.lockIndex != -1)
		eye.lockIndex = 0;

	if (eye.lockIndex != -1) {
		GravityBody* body = &frameBodies[eye.lockIndex];
		if (body->parentIndex != -1 && overheadLock) {
			eye.right = glm::normalize(body->position - frameBodies[body->parentIndex].position);
			eye.up = glm::normalize(body->velocity - frameBodies[body->parentIndex].velocity);
			eye.direction = glm::cross(eye.up, eye.right);
			eye.right = glm::cross(eye.direction, eye.up);
		}
		eye.position = body->position - eye.lockDistanceFactor * body->radius * eye.direction;
	}
}

void updateGravCam(Camera& eye, double deltaTime) {
	if (hasPhysics) {
		GravityBody* camBody = &frameBodies[frameBodies.size() - 1];
		// bind camera to gravity-bound object
		eye.position = camBody->position;
		camBody->velocity += eye.velocity;
		eye.velocity = glm::dvec3(0);

		//// project camera object's future movement in the system
		//std::vector<GravityBody> copy = bodies;
		//for (int i = 0; i < cameraFutureTrailLength; i++) {
		//	updateBodies(deltaTime, copy);
		//	bodies[bodies.size() - 1].trail->push_back(copy[copy.size() - 1].position);
		//}
	}
}

void updateCamera(Camera& eye, double deltaTime) {
	switch (eye.mode) {
	case FREE_CAM:
		updateFreeCam(eye, deltaTime);
		break;
	case LOCK_CAM:
		updateLockCam(eye, deltaTime);
		break;
	case GRAV_CAM:
		updateGravCam(eye, deltaTime);
		break;
	}
}

void drawQuad() {
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glDrawArrays(GL_TRIANGLES, 0, 6); // 6 vertices for a quad
	glBindVertexArray(0);
}

void renderBillboards() {
	// configure shader
	glUseProgram(spriteShader.index);

	// update camera
	glm::mat4 view = glm::mat4(glm::mat3(camera.viewMatrix()));
	updateProjectionMatrix();

	setPV(spriteShader, projection, view);
	glm::ivec2 windowSize(windowWidth, windowHeight);
	glUniform2iv(spriteShader.uniforms[WINDOW_SIZE], 1, &windowSize[0]);

	// assign points in space for star billboards 
	// and get the number of billboards to be rendered
	GLsizei numStars = updateStarPositions();

	// load star sprite
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, starTex);
	glUniform1i(spriteShader.uniforms[TEX_MAP], 0);

	// send texture metadata
	glm::ivec2 dims;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &dims.x);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &dims.y);
	glUniform2iv(spriteShader.uniforms[TEX_SIZE], 1, &dims[0]);

	// draw billboards at the configured locations
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, numStars);
}

void render(Camera& camera) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Activate the shader program
	glUseProgram(shader.index);
	glm::vec3 lightPos = frameBodies[0].position;
	glm::vec3 lightColor = glm::vec3(1.0f);
	glUniform3fv(shader.uniforms[LIGHT_POS], 1, &lightPos[0]);
	glUniform3fv(shader.uniforms[LIGHT_COLOR], 1, &lightColor[0]);

	Camera copy = camera;
	for (GravityBody body : frameBodies) {
		if (testObjectVisibility(body, camera)) {
			copy.position = camera.position - body.position;
			glm::mat4 relativeView = copy.viewMatrix();
			glUniform3fv(shader.uniforms[LIGHT_POS], 1, &(lightPos - glm::vec3(body.position))[0]);
			setPV(shader, projection, relativeView);
			glm::dvec3 bodyPos = body.position;
			body.position = glm::dvec3(0.0);
			body.draw(shader, MODE_TEX);
			body.position = bodyPos;
		}
	}

	// start trails

	// set camera
	glm::mat4 view = camera.viewMatrix();
	setPV(shader, projection, view);
	trailVertices.clear();
	trailAlphas.clear();

	glUseProgram(trailShader.index);
	setPV(trailShader, projection, view);
	for (GravityBody body : frameBodies) {
		// axis
		glm::dvec3 axisOfRotation(0.0, 1.0, 0.0);
		axisOfRotation = body.getRotationQuat() * axisOfRotation;

		trailVertices.push_back(body.position + 2 * body.radius * axisOfRotation);
		trailVertices.push_back(body.position - 2 * body.radius * axisOfRotation);
		trailAlphas.push_back(1.0f);
		trailAlphas.push_back(1.0f);

		if (body.trail) {
			// orbit
			trailVertices.insert(trailVertices.end(), body.trail->begin(), body.trail->end());
			for (size_t j = body.trail->size(); j > 0; j--) {
				trailAlphas.push_back(1.0f);
			}
		}
	}

	glBindVertexArray(trailVAO);
	glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
	if (trailVertices.size() * sizeof(glm::vec3) > trailCapacity) {
		glBufferData(GL_ARRAY_BUFFER, trailVertices.size() * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
		trailCapacity = (GLsizei)(trailVertices.size() * sizeof(glm::vec3));
	}
	glBufferSubData(GL_ARRAY_BUFFER, 0, trailVertices.size() * sizeof(glm::vec3), trailVertices.data());

	glEnableVertexAttribArray(0); // Assuming location 0 for positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

	glBindBuffer(GL_ARRAY_BUFFER, trailAlphaBuf);
	glBufferData(GL_ARRAY_BUFFER, trailAlphas.size() * sizeof(float), trailAlphas.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(1); // Assuming location 1 for alphas
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), 0);

	GLint offset = 0;

	for (GravityBody body : frameBodies) {
		glm::mat4 modelMatrix(1.0f);
		glm::vec3 color(1.0f);
		if (body.trail)
			color = body.trail->color;

		glUniform3fv(trailShader.uniforms[OBJ_COLOR], 1, &color[0]);

		// axis
		glUniformMatrix4fv(trailShader.M, 1, GL_FALSE, &modelMatrix[0][0]);
		glDrawArrays(GL_LINE_STRIP, offset, 2);
		offset += 2;

		if (body.trail) {
			size_t parentIndex = body.trail->parentIndex;

			// orbit
			if (body.parentIndex != -1) {
				glm::mat4 rotate = relativeRotationalMatrix(&body, &frameBodies[parentIndex], true);

				modelMatrix = glm::translate(glm::dmat4(1.0), frameBodies[body.parentIndex].position);
				modelMatrix *= rotate;
			}

			glUniformMatrix4fv(trailShader.M, 1, GL_FALSE, &modelMatrix[0][0]);

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

void renderPIP() {
	// update frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, pipFBO);
	GLsizei pipWidth = GLsizei(windowWidth * pipSize);
	GLsizei pipHeight = GLsizei(windowHeight * pipSize);
	glViewport(0, 0, pipWidth, pipHeight);

	// update texture buffer
	glBindTexture(GL_TEXTURE_2D, pipTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pipWidth, pipHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pipTexture, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, pipDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, pipWidth, pipHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pipDepthBuffer);

	render(pipCam);

	// return to screen buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
	updateProjectionMatrix();

	// render frame buffer contents to the screen
	glUseProgram(frameShader.index);

	// adjust quad shape to occupy bottom right corner
	glm::mat4 pipTransform = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f - pipSize, -1.0f + pipSize, 0.0f));
	pipTransform = glm::scale(pipTransform, glm::vec3(pipSize, pipSize, 1.0f));
	glUniformMatrix4fv(frameShader.M, 1, GL_FALSE, &pipTransform[0][0]);
	glBindTexture(GL_TEXTURE_2D, pipTexture);

	drawQuad();  // Assume a quad is already defined for rendering
}

static bool showWelcomeMenu = true;
static bool showLockIndexMenu = true;

void drawGUI(ImGuiIO& io) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();
	
	float padding = 10.0f;
	ImVec2 w1Size, w1Pos;
	// control menu
	if (showWelcomeMenu) {
		ImGui::SetNextWindowPos(ImVec2(padding, padding));
		ImGui::Begin("Welcome to N-Body Simulator", &showWelcomeMenu, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::TextWrapped("Controls:");
		ImGui::BulletText("WASD - Move the camera through space");
		ImGui::BulletText("Mouse - Pitch/yaw the camera");
		ImGui::BulletText("Q/E - Roll the camera");
		ImGui::BulletText("F - Switch between free cam and locked cam");
		ImGui::BulletText("L - Toggle overhead lock on locked cam");
		ImGui::BulletText("P - Toggle physics simulation");
		ImGui::BulletText("[ / ] - Increment up / down celestial bodies");
		ImGui::BulletText("G - Snap to current target");
		ImGui::BulletText("< / > - Adjust simulation speed");
		ImGui::BulletText("Esc - Exit the program");

		ImGui::Separator();

		if (ImGui::Button("Close")) {
			showWelcomeMenu = false;
		}

		ImGui::SameLine();

		if (ImGui::Button("Open Settings")) {
			showLockIndexMenu = true;
		}
		w1Size = ImGui::GetWindowSize();
		w1Pos = ImGui::GetWindowPos();
		ImGui::End();
	}
	else {
		w1Pos = ImVec2(padding, 0.0f);
	}

	// target selector menu
	if (showLockIndexMenu) {
		ImGui::SetNextWindowPos(ImVec2(w1Pos.x, w1Pos.y + w1Size.y + padding));
		ImGui::Begin("Camera Settings", &showLockIndexMenu, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Text("Select a body of interest:");

		// user access to change camera's lockIndex
		int index = (int)std::max((size_t)0, std::min(camera.lockIndex, frameBodies.size() - 1));
		ImGui::SliderInt("ID", &index, 0, (int)frameBodies.size() - 1);
		ImGui::InputInt("Manual Entry", &index);
		
		camera.lockIndex = std::max((size_t)0, std::min((size_t)index, frameBodies.size() - 1));

		ImGui::Separator();

		if (ImGui::Button("Close")) {
			showLockIndexMenu = false;
		}

		ImGui::End();
	}

	// render dialogs
	ImGui::EndFrame();
	ImGui::UpdatePlatformWindows();
	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();
	ImGui_ImplOpenGL3_RenderDrawData(draw_data);
}

void renderLoop() {
	double lastFrameTime = glfwGetTime();
	double currentTime = glfwGetTime();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	while (!glfwWindowShouldClose(window)) {
		if (currentTime - lastFrameTime > MIN_FRAME_TIME) {
			// capture physics results when they are ready
			{
				std::unique_lock<std::mutex> lock(physicsMutex);

				physicsCV.wait(lock, [] { return physicsUpdated; });
				physicsUpdated = false;

				frameBodies = bodies;

				lock.unlock();
			}

			updateCamera(camera, currentTime - lastFrameTime);
			updateCamera(pipCam, currentTime - lastFrameTime);

			if (hasPhysics)
				updateTrails();

			render(camera);

			//if (hasPhysics && camera.mode == GRAV_CAM) {
			//	// clean up future trail
			//	for (int i = 0; i < cameraFutureTrailLength; i++)
			//		bodies[bodies.size() - 1].trail->pop_back();
			//}

			renderPIP();
			renderBillboards();

			lastFrameTime = currentTime;

			drawGUI(io);

			glfwSwapBuffers(window);
			glfwPollEvents();
		}

		currentTime = glfwGetTime();
	}
}