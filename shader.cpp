#include "shader.h"
#include <iostream>

// Function to compile and link shaders
GLuint compileShader(const char* vertSource, const char* fragSource) {
	GLint success;

	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, &vertSource, nullptr);
	glCompileShader(vertShader);

	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragSource, nullptr);
	glCompileShader(fragShader);

	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(vertShader, 512, nullptr, infoLog);
		std::cerr << "Vertex Shader Compilation Error: " << infoLog << std::endl;
	}
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(fragShader, 512, nullptr, infoLog);
		std::cerr << "Fragment Shader Compilation Error: " << infoLog << std::endl;
	}

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertShader);
	glAttachShader(shaderProgram, fragShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
		std::cerr << "Program Linking Error: " << infoLog << std::endl;
	}

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return shaderProgram;
}

Shader initStandardShader() {
	const char* vertexSource = R"(
		#version 330 core
		layout (location = 0) in vec3 aPos;
		layout (location = 1) in vec3 aNormal;
		layout (location = 2) in vec2 aTex;

		out vec3 FragPos;
		out vec3 Normal;
		out vec2 TexCoords;

		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

		void main() {
			FragPos = vec3(model * vec4(aPos, 1.0));
			Normal = mat3(transpose(inverse(model))) * aNormal;
			TexCoords = aTex;
			gl_Position = projection * view * vec4(FragPos, 1.0);
		}
	)";

	const char* fragmentSource = R"(
		#version 330 core
		out vec4 FragColor;

		in vec3 FragPos;
		in vec3 Normal;
		in vec2 TexCoords;

		uniform vec3 objectColor;
		uniform vec4 material;
		uniform vec3 lightColor;
		uniform vec3 lightPos;
		uniform vec3 viewPos;
		uniform sampler2D textureMap;
		uniform int usesTexture;

		void main() {
			vec4 texColor = texture(textureMap, TexCoords);

			// Ambient
			float amb = material.x;

			// Diffuse
			vec3 norm = normalize(Normal);
			vec3 lightDir = normalize(lightPos - FragPos);
			float diff = material.y * max(dot(norm, lightDir), 0.0);

			// Specular
			vec3 viewDir = normalize(viewPos - FragPos);
			vec3 reflectDir = reflect(-lightDir, norm);
			vec3 halfwayDir = normalize(lightDir + viewDir);  
			float spec = material.z * pow(max(dot(norm, halfwayDir), 0.0), 32);

			float emission = material.w;

			// Final color
			vec3 result = ((amb + diff + spec) * lightColor + emission) * objectColor;
			if (usesTexture == 1) {
				FragColor = texColor * vec4(result, 1.0);
			}
			else {
				FragColor = vec4(result, 1.0);
			}
		}
	)";

	GLuint shaderProgram = compileShader(vertexSource, fragmentSource);

	Shader shader;
	glUseProgram(shaderProgram);
	shader.index = shaderProgram;
	shader.M = glGetUniformLocation(shaderProgram, "model");
	shader.V = glGetUniformLocation(shaderProgram, "view");
	shader.P = glGetUniformLocation(shaderProgram, "projection");
	shader.lightPos = glGetUniformLocation(shaderProgram, "lightPos");
	shader.lightColor = glGetUniformLocation(shaderProgram, "lightColor");
	shader.objectColor = glGetUniformLocation(shaderProgram, "objectColor");
	shader.objectMat = glGetUniformLocation(shaderProgram, "material");
	shader.viewPos = glGetUniformLocation(shaderProgram, "viewPos");
	shader.texBool = glGetUniformLocation(shaderProgram, "usesTexture");

	return shader;
}

Shader initSkyboxShader() {
	const char* vertexSource = R"(
		#version 330 core
		layout (location = 0) in vec3 aPos;

		out vec3 TexCoords;

		uniform mat4 projection;
		uniform mat4 view;

		void main()
		{
			TexCoords = aPos;
			vec4 pos = projection * view * vec4(aPos, 1.0);
			gl_Position = pos.xyww;  // Keep the depth at the maximum value
		}  
	)";

	const char* fragmentSource = R"(
		#version 330 core
		out vec4 FragColor;

		in vec3 TexCoords;

		uniform samplerCube skybox;

		void main()
		{    
			FragColor = texture(skybox, TexCoords);
			//FragColor = vec4(1.0);
		}
	)";

	GLuint shaderProgram = compileShader(vertexSource, fragmentSource);

	Shader shader;
	glUseProgram(shaderProgram);
	shader.index = shaderProgram;
	shader.V = glGetUniformLocation(shaderProgram, "view");
	shader.P = glGetUniformLocation(shaderProgram, "projection");

	return shader;
}
