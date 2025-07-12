#include "render.h"
#include "controls.h"
#include <mutex>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

GLFWwindow* window;
glm::mat4 projection;
int screenSize = windowWidth;
float pipSize = 0.35f;

ImGuiStyle* style;

std::mutex physicsMutex;

size_t cameraFutureTrailLength = 2500;
GLsizei starCapacity = 20;
GLsizei trailCapacity = 0;

GLuint trailVAO, trailVBO, trailAlphaBuf, quadVAO, quadVBO, pipFBO, pipTexture, pipDepthBuffer, instanceVBO, starTex;
ImFont *defaultFont, *largeFont;
Shader shader, skyboxShader, trailShader, frameShader, spriteShader;

std::vector<glm::vec3> trailVertices;
std::vector<float> trailAlphas;

std::vector<std::shared_ptr<Entity>> frameEntities;

void setPV(Shader& shader, glm::mat4& P, glm::mat4& V) {
	glUniformMatrix4fv(shader.P, 1, GL_FALSE, &P[0][0]);
	glUniformMatrix4fv(shader.V, 1, GL_FALSE, &V[0][0]);
}

// update projection matrix based on window size
void updateProjectionMatrix(Camera& camera, GLFWwindow* window) {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	if (height == 0) height = 1; // prevent division by zero

	float aspect = (float)width / (float)height;
	projection = glm::perspective(camera.FOV * height / screenSize, aspect, 1e-1f, 1e9f);
}

void initWindow() {
	if (!glfwInit()) {
		fprintf(stderr, "GLFW initialization failed!\n");
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_DEPTH_BITS, 32);

	// multisample buffer for antialiasing
	glfwWindowHint(GLFW_SAMPLES, 4);

	window = glfwCreateWindow(windowWidth, windowHeight, "N-Body Simulator", nullptr, nullptr);

	if (!window) {
		fprintf(stderr, "Window creation failed!\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	GLFWmonitor* screen = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(screen);
	glfwSetWindowPos(window, (mode->width - windowWidth) / 2, (mode->height - windowHeight) / 2);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int width, int height) {
		glViewport(0, 0, width, height);
		});

	glewInit();

	// GLFW interrupt response
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetKeyCallback(window, key_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	setXY(window);
}

// Set this function as a callback to update projection matrix during window resizing
void static window_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);  // Set the OpenGL viewport to match the new window size
	windowWidth = width;
	windowHeight = height;
	updateProjectionMatrix(camera, window);  // Update the projection matrix with the new size
}

void initCamera() {
	glm::dvec3 direction = glm::normalize(bodies[bodies.size() - 1]->position - bodies[0]->position);
	glm::dvec3 right(1.0, 0.0, 0.0);
	glm::dvec3 up = glm::cross(right, direction);

	camera = Camera(
		bodies[bodies.size() - 1]->position,
		direction, up
	);
	camera.mode = LOCK_CAM;
	camera.atIndex = camera.eyeIndex = 0;

	// obtain initial perspective information from relationship between window and screen
	GLFWmonitor* screen = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(screen);
	screenSize = mode->height;
	projection = glm::perspective(camera.FOV * windowHeight / screenSize, (float)windowWidth / windowHeight, 1e-1f, 1e9f);
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
GLsizei updateStarPositions(Camera& camera) {
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

	std::vector<glm::vec3> positions;
	positions.reserve(frameBodies.size());
	for (std::shared_ptr<GravityBody> body : frameBodies) {
		if (body->mass > 1e5f)
			positions.push_back(body->position - camera.position);
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

	for (std::shared_ptr<GravityBody> body : bodies) {
		GLuint tex = body->surface.getTexture();
		GLuint nor = body->surface.getNormalMap();
		if (tex)
			glDeleteTextures(1, &tex);
		if (nor)
			glDeleteTextures(1, &nor);
	}

	glfwTerminate();
}

bool testObjectVisibility(size_t index, Camera& camera) {
	if (camera.mode == LOCK_CAM && index == camera.eyeIndex && index != camera.atIndex)
		return false;
	double longestPossibleAxisLength = glm::length(frameBodies[index]->scale);
	double angularSize = 2.0 * atan2(longestPossibleAxisLength, glm::distance(camera.position, frameBodies[index]->position));
	if (windowHeight * angularSize / camera.FOV < 0.5) {
		return false;
	}
	return true;
}

void updateFreeCam(Camera& eye) {
	if (&eye == &camera)
		flyCam(window);
}

void updateLockCam(Camera& eye) {
	rollCamera(eye, window);

	if (eye.atIndex >= frameBodies.size() && eye.atIndex != -1)
		eye.atIndex = 0;

	if (eye.atIndex != -1) {
		std::shared_ptr<GravityBody> body = frameBodies[eye.atIndex];
		if (eye.eyeIndex == -1 || eye.eyeIndex == eye.atIndex) {
			if (body->parentIndex != -1 && overheadLock) {
				eye.right = glm::normalize(body->position - frameBodies[body->parentIndex]->position);
				eye.up = glm::normalize(body->velocity - frameBodies[body->parentIndex]->velocity);
				eye.direction = glm::cross(eye.up, eye.right);
				eye.right = glm::cross(eye.direction, eye.up);
			}
			eye.position = body->position - eye.lockDistanceFactor * body->radius * eye.direction;
		}
		else {
			eye.watchFrom(body, frameBodies[eye.eyeIndex]);
		}
	}
}

void updateGravCam(Camera& eye) {
	if (hasPhysics) {
		std::shared_ptr<GravityBody> camBody = bodies[eye.eyeIndex];
		eye.position = camBody->position;

		if (&eye == &camera) {
			flyCam(window);

			bodies[eye.eyeIndex]->position += eye.velocity * deltaTime;
			bodies[eye.eyeIndex]->velocity += eye.velocity;
		}
	}
}

void updateCamera(Camera& eye) {
	switch (eye.mode) {
	case FREE_CAM:
		updateFreeCam(eye);
		break;
	case LOCK_CAM:
		updateLockCam(eye);
		break;
	case GRAV_CAM:
		updateGravCam(eye);
		break;
	}
}

void drawQuad() {
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glDrawArrays(GL_TRIANGLES, 0, 6); // 6 vertices for a quad
	glBindVertexArray(0);
}

void renderBillboards(Camera& camera, glm::ivec2 windowSize) {
	// configure shader
	glUseProgram(spriteShader.index);

	// update camera
	glm::mat4 view = glm::mat4(glm::mat3(camera.viewMatrix()));
	updateProjectionMatrix(camera, window);

	setPV(spriteShader, projection, view);
	glUniform2iv(spriteShader.uniforms[WINDOW_SIZE], 1, &windowSize[0]);

	// assign points in space for star billboards 
	// and get the number of billboards to be rendered
	GLsizei numStars = updateStarPositions(camera);

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

	glDepthFunc(GL_LEQUAL);
	glUseProgram(skyboxShader.index);

	// set camera
	glm::mat4 view = camera.viewMatrix();
	
	glm::mat4 infiniteView = glm::mat4(glm::mat3(view));
	setPV(skyboxShader, projection, infiniteView);

	Entity::skybox->draw(skyboxShader, MODE_CUBEMAP);
	glDepthFunc(GL_LESS);

	// Activate the shader program
	glUseProgram(shader.index);
	glm::vec3 lightPos = frameBodies[0]->position;
	glm::vec3 lightColor = glm::vec3(1.0f);
	glUniform3fv(shader.uniforms[LIGHT_POS], 1, &lightPos[0]);
	glUniform3fv(shader.uniforms[LIGHT_COLOR], 1, &lightColor[0]);

	// render gravity bodies
	for (size_t i = 0; i < frameBodies.size(); i++) {
		if (testObjectVisibility(i, camera)) {
			glm::dvec3 bodyPos = frameBodies[i]->position;
			glm::mat4 relativeViewMat = camera.viewMatrix(camera.position - bodyPos);
			glm::vec3 relativeViewPos = camera.position - bodyPos;
			glUniform3fv(shader.uniforms[VIEW_POS], 1, &relativeViewPos[0]);
			glUniform3fv(shader.uniforms[LIGHT_POS], 1, &(lightPos - glm::vec3(bodyPos))[0]);
			setPV(shader, projection, relativeViewMat);
			frameBodies[i]->draw(shader, MODE_TEX);
		}
	}
	
	// get entities in order of increasing proximity to the camera
	std::map<double, std::shared_ptr<Entity>> orderedEntities;
	for (size_t i = 0; i < frameEntities.size(); i++) {
		glm::dvec3 wPos = frameEntities[i]->position + frameEntities[i]->root->position;
		double distance = glm::length(wPos - camera.position);
		orderedEntities[distance] = frameEntities[i];
	}

	for (auto it = orderedEntities.rbegin(); it != orderedEntities.rend(); it++) {
		std::shared_ptr<Entity> entity = it->second;
		glm::dvec3 bodyPos = entity->position + entity->root->position;
		
		glm::mat4 relativeView = camera.viewMatrix(camera.position - bodyPos);
		glUniform3fv(shader.uniforms[LIGHT_POS], 1, &(lightPos - glm::vec3(bodyPos))[0]);
		setPV(shader, projection, relativeView);

		glm::dmat4 rootMatrix = glm::dmat4(1.0);
		if (entity->root) {
			rootMatrix = glm::dmat4(entity->root->rotQuat);
		}

		entity->draw(shader, MODE_TEX, rootMatrix);
	}
	
	// start trails
	if (doTrails) {
		// set camera
		setPV(shader, projection, view);
		trailVertices.clear();
		trailAlphas.clear();

		glUseProgram(trailShader.index);
		setPV(trailShader, projection, view);
		for (size_t i = 0; i < frameBodies.size(); i++) {
			GravityBody body = *frameBodies[i];

			// axis
			if (testObjectVisibility(i, camera)) {
				glm::dvec3 axisOfRotation(0.0, 1.0, 0.0);
				axisOfRotation = body.rotQuat * axisOfRotation;

				trailVertices.push_back(body.position + 2 * body.radius * axisOfRotation);
				trailVertices.push_back(body.position - 2 * body.radius * axisOfRotation);
				trailAlphas.push_back(1.0f);
				trailAlphas.push_back(1.0f);
			}

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

		for (size_t i = 0; i < frameBodies.size(); i++) {
			GravityBody body = *frameBodies[i];

			glm::mat4 modelMatrix(1.0f);
			glm::vec3 color(1.0f);
			if (body.trail)
				color = body.trail->color;

			glUniform3fv(trailShader.uniforms[OBJ_COLOR], 1, &color[0]);

			// axis
			if (testObjectVisibility(i, camera)) {
				glUniformMatrix4fv(trailShader.M, 1, GL_FALSE, &modelMatrix[0][0]);
				glDrawArrays(GL_LINE_STRIP, offset, 2);
				offset += 2;
			}

			if (body.trail) {
				size_t parentIndex = body.trail->parentIndex;

				// orbit
				if (body.parentIndex != -1) {
					glm::mat4 rotate = relativeRotationalMatrix(frameBodies, i, parentIndex, true);

					modelMatrix = glm::translate(glm::dmat4(1.0), frameBodies[body.parentIndex]->position);
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
	}
	// end trails
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

	updateProjectionMatrix(pipCam, window);
	render(pipCam);
	renderBillboards(pipCam, glm::ivec2(pipSize * windowWidth, pipSize * windowHeight));

	// return to screen buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);

	// render frame buffer contents to the screen
	glUseProgram(frameShader.index);

	// adjust quad shape to occupy bottom right corner
	glm::mat4 pipTransform = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f - pipSize, -1.0f + pipSize, 0.0f));
	pipTransform = glm::scale(pipTransform, glm::vec3(pipSize, pipSize, 1.0f));
	glUniformMatrix4fv(frameShader.M, 1, GL_FALSE, &pipTransform[0][0]);
	glBindTexture(GL_TEXTURE_2D, pipTexture);

	drawQuad();  // Assume a quad is already defined for rendering
}

void drawGUI(ImGuiIO& io) {
	//io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();
	
	float padding = 10.0f;
	ImVec2 w1Size, w1Pos;
	ImVec2 w2Size, w2Pos;

	// control menu
	if (showWelcomeMenu) {
		ImGui::SetNextWindowPos(ImVec2(padding, padding));
		ImGui::Begin("Welcome to N-Body Simulator", &showWelcomeMenu, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::TextWrapped("Controls:");
		ImGui::BulletText("WASD - Move the camera through space");
		ImGui::BulletText("Mouse - Pitch/yaw the camera");
		ImGui::BulletText("Q/E - Roll the camera");
		ImGui::BulletText("P - Toggle physics simulation");
		ImGui::BulletText("< / > - Adjust simulation speed");
		ImGui::BulletText("C - Cycle between locked, free, and grav cam");
		ImGui::BulletText("L - Toggle overhead lock on locked cam");
		ImGui::BulletText("T - Toggle trails");
		ImGui::BulletText("[ / ] - Increment up / down celestial bodies");
		ImGui::BulletText("G - Snap to current target");
		ImGui::BulletText("F12 - Toggle these menus");
		ImGui::BulletText("Esc - Exit the program");

		ImGui::Separator();

		if (ImGui::Button("Close")) {
			showWelcomeMenu = false;
		}

		ImGui::SameLine();

		if (ImGui::Button("Open Camera Controls")) {
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

		const char* windowTitle;
		switch (camera.mode) {
		case LOCK_CAM:
			windowTitle = "Locked Camera";
			break;
		case FREE_CAM:
			windowTitle = "Free Camera";
			break;
		case GRAV_CAM:
			windowTitle = "Gravity-Based Camera";
			break;
		default:
			windowTitle = "";
		}

		ImVec2 titleSize = ImGui::CalcTextSize(windowTitle);
		ImGui::SetNextWindowSize(ImVec2(fmax(w1Size.x, 400.0f), w2Size.y));

		ImGui::Begin(windowTitle, &showLockIndexMenu, 
			ImGuiWindowFlags_NoResize | 
			ImGuiWindowFlags_AlwaysAutoResize);

		if (camera.mode == LOCK_CAM) {
			ImGui::Text("Select a body of interest:");

			// user access to change a locked camera's position and target
			int numBodies = (int)frameBodies.size() - 2;
			int atIndex = std::max(0, std::min((int)camera.atIndex, numBodies));
			int eyeIndex = std::max(0, std::min((int)camera.eyeIndex, numBodies));
			ImGui::SliderInt("Target", &atIndex, 0, numBodies);
			ImGui::SliderInt("Observer", &eyeIndex, 0, numBodies);

			camera.atIndex = std::max(0, std::min(atIndex, numBodies));
			camera.eyeIndex = std::max(0, std::min(eyeIndex, numBodies));
		}
		else if (camera.mode == FREE_CAM) {
			ImGui::Text("X: %11.3e\nY: %11.3e\nZ: %11.3e", 
				camera.position.x, camera.position.y, camera.position.y);
		}
		else if (camera.mode == GRAV_CAM) {
			ImGui::Text("X: %11.3e\nY: %11.3e\nZ: %11.3e\nVel: %11.3e", 
				camera.position.x, camera.position.y, camera.position.y,
				glm::length(frameBodies[camera.eyeIndex]->velocity));
		}

		ImGui::Separator();

		if (ImGui::Button("Close")) {
			showLockIndexMenu = false;
		}

		w2Size = ImGui::GetWindowSize();
		w2Pos = ImGui::GetWindowPos();

		ImGui::End();
	}

	// time display
	{
		ImGui::SetNextWindowPos(ImVec2(padding, w2Pos.y + w2Size.y + padding));

		ImGui::Begin("Elapsed Time Display", nullptr,
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoResize | 
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoTitleBar);

		ImGui::Text("Elapsed time: %.1f yrs", elapsedTime / 86400 / 365.25);
		ImGui::Text("Time Step: %.3f s", frameTime);
		ImGui::Text("Earth dtp: %.3e", glm::distance(frameBodies[1]->position, frameBodies[0]->position));

		ImGui::End();
	}
	
	// settings controller
	if (showSettingsMenu) {
		ImGui::Begin("Settings", &showSettingsMenu, 
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_AlwaysAutoResize);

		float timeStepLog = (float)log10(timeStep);

		ImGui::Checkbox("Physics", &hasPhysics);
		ImGui::Text("Time Step (Logarithmic)");
		ImGui::SliderFloat("##timestep", &timeStepLog, 0, 10);
		ImGui::Checkbox("Trails", &doTrails);

		ImGui::SetWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - ImGui::GetWindowSize().x - padding, padding), ImGuiCond_Always);

		ImGui::End();

		timeStep = pow(10.0, (double)timeStepLog);
	}
	else {
		ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 50 - padding, padding), ImGuiCond_Always);

		ImGui::PushFont(largeFont);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

		// Create a window without a title or background, to act as a button container
		ImGui::Begin("Settings Button", nullptr, 
			ImGuiWindowFlags_NoTitleBar | 
			ImGuiWindowFlags_NoResize | 
			ImGuiWindowFlags_NoMove | 
			ImGuiWindowFlags_NoDecoration);

		const char* settingsIcon = "\xE2\x9A\x99";  // UTF-8 encoding for ⚙

		if (ImGui::Button(settingsIcon, ImVec2(50,50))) {
			showSettingsMenu = true;
		}

		ImGui::SetWindowSize(ImVec2(50, 50));

		ImGui::End();

		ImGui::PopFont();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
	}

	// render dialogs
	ImGui::EndFrame();
	ImGui::UpdatePlatformWindows();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
}

void renderLoop() {
	double lastFrameTime = glfwGetTime();
	double currentTime = glfwGetTime();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	const char* fontPath = "../../assets/DejaVuSansMono.ttf";

	ImVector<ImWchar> ranges;
	ImFontGlyphRangesBuilder builder;
	builder.AddRanges(io.Fonts->GetGlyphRangesDefault());                        // Add a string (here "Hello world" contains 7 unique characters)
	builder.AddChar(0x2699);                               // Add a specific character
	builder.BuildRanges(&ranges);                          // Build the final result (ordered ranges with all the unique characters submitted)

	defaultFont = io.Fonts->AddFontFromFileTTF(fontPath, 16.0f, nullptr, ranges.Data);
	largeFont = io.Fonts->AddFontFromFileTTF(fontPath, 32.0f, nullptr, ranges.Data);\

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	style = &ImGui::GetStyle();
	style->GrabRounding = 5.0f;
	style->FrameRounding = 5.0f;
	style->WindowRounding = 5.0f;
	style->Colors[ImGuiCol_Text] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);

	try {
		while (!glfwWindowShouldClose(window)) {
			if (currentTime - lastFrameTime > MIN_FRAME_TIME) {
				// capture physics results when they are ready
				{
					std::unique_lock<std::mutex> lock(physicsMutex);

					physicsCV.wait(lock, [] { return physicsUpdated; });
					physicsUpdated = false;

					if (!doTrails && trailVertices.size() > 0) {
						for (std::shared_ptr<GravityBody> body : bodies) {
							if (body->trail) {
								body->trail->queue.clear();
							}
						}
						trailVertices = std::vector<glm::vec3>();
						trailAlphas = std::vector<float>();
					}

					frameBodies.clear();
					frameEntities.clear();
					for (std::shared_ptr<GravityBody> body : bodies) {
						frameBodies.push_back(std::make_shared<GravityBody>(*body));
						for (std::shared_ptr<Entity> entity : entities) {
							if (entity->root == body) {
								frameEntities.push_back(std::make_shared<Entity>(*entity));
								frameEntities[frameEntities.size() - 1]->root = frameBodies[frameBodies.size() - 1];
							}
						}
					}

					lock.unlock();
				}

				deltaTime = currentTime - lastFrameTime;
				updateCamera(camera);
				updateCamera(pipCam);

				if (hasPhysics && doTrails)
					updateTrails(frameBodies);
				render(camera);

				//if (hasPhysics && camera.mode == GRAV_CAM) {
				//	// clean up future trail
				//	for (int i = 0; i < cameraFutureTrailLength; i++)
				//		bodies[bodies.size() - 1].trail->pop_back();
				//}

				renderPIP();
				renderBillboards(camera, glm::ivec2(windowWidth, windowHeight));

				lastFrameTime = currentTime;

				drawGUI(io);

				glfwSwapBuffers(window);
				glfwPollEvents();
			}

			currentTime = glfwGetTime();
		}
	}
	catch (std::exception e) {
		printf("Render err\n");
	}
}