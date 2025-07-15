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
		layout (location = 3) in vec3 aTan;
		layout (location = 4) in vec3 aBitan;

		out vec3 FragPos;
		out vec3 Normal;
		out vec3 Tangent;
		out vec3 Bitangent;
		out vec2 TexCoords;
		out float logDepth;

		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;
		uniform float oblateness;

		void main() {
			FragPos = vec3(model * vec4(aPos.x, aPos.y * (1.0 - oblateness), aPos.z, 1.0));
			vec4 ViewPos = view * vec4(FragPos, 1.0);
			
			Tangent = normalize(vec3(model * vec4(aTan, 0.0)));
			Bitangent = normalize(vec3(model * vec4(aBitan, 0.0)));
			Normal = normalize(vec3(model * vec4(aNormal, 0.0)));

			TexCoords = aTex;
			gl_Position = vec4(projection * ViewPos);
			
			if (ViewPos.z < 0) {
				logDepth = log(1.0 - ViewPos.z) / log(1e9 + 1.0);
			}
			else {
				logDepth = 0.0;
			}
		}
	)";

	const char* fragmentSource = R"(
		#version 330 core
		in vec3 FragPos;
		in vec3 Normal;
		in vec3 Tangent;
		in vec3 Bitangent;
		in vec2 TexCoords;
		in float logDepth;

		out vec4 FragColor;

		uniform vec3 objectColor;
		uniform vec4 material;
		uniform vec3 lightColor;
		uniform vec3 lightPos;
		uniform float lightRadius;
		uniform vec3 viewPos;
		uniform vec4 occluders[8];
		uniform int numOccluders;
		uniform sampler2D textureMap;
		uniform sampler2D normalMap;
		uniform int usesTexture;
		uniform int usesNormalMap;
		
		float PI = 3.141592653589793;

		float eclipseFactor() {
			float thetaLight = asin(lightRadius / length(lightPos - FragPos));
			float areaLight = PI * thetaLight * thetaLight;
			float totalOcclusion = 0.0;

			for (int i = 0; i < numOccluders; i++) {
				vec3 occluderPos = occluders[i].xyz;
				float occluderRadius = occluders[i].w;

				float thetaOccluder = asin(occluderRadius / length(occluderPos - FragPos));
				float phi = acos(dot(normalize(lightPos - FragPos), normalize(occluderPos - FragPos)));

				if (phi >= thetaLight + thetaOccluder)
					continue; // no overlap

				float A = 0.0;

				if (phi <= abs(thetaLight - thetaOccluder)) {
					A = PI * min(thetaLight, thetaOccluder) * min(thetaLight, thetaOccluder); // full occlusion
				} else {
					float r1 = thetaLight;
					float r2 = thetaOccluder;
					float d = phi;

					float alpha = acos((d*d + r1*r1 - r2*r2) / (2.0 * d * r1));
					float beta = acos((d*d + r2*r2 - r1*r1) / (2.0 * d * r2));
					float segment = 0.5 * sqrt((-d + r1 + r2)*(d + r1 - r2)*(d - r1 + r2)*(d + r1 + r2));
					A = r1*r1*alpha + r2*r2*beta - segment;
				}

				totalOcclusion += A;
			}

			float occlusionFraction = clamp(totalOcclusion / areaLight, 0.0, 1.0);
			return 1.0 - occlusionFraction; // used to modulate lighting
		}

		void main() {
			gl_FragDepth = logDepth;

			vec4 texColor = texture(textureMap, TexCoords);

			// Ambient
			float amb = material.x;

			// Diffuse
			vec3 norm;
			if (usesNormalMap == 1) {
				norm = texture(normalMap, TexCoords).rgb;
				norm = norm * 2.0 - 1.0;
				mat3 TBN = mat3(Tangent, Bitangent, Normal);
				norm = normalize(TBN * norm);
			}
			else {
				norm = normalize(Normal);
			}

			vec3 lightDir = normalize(lightPos - FragPos);
			float diff = material.y * max(dot(norm, lightDir), 0.0);

			// Specular
			vec3 viewDir = normalize(viewPos - FragPos);
			vec3 reflectDir = reflect(-lightDir, norm);
			vec3 halfwayDir = normalize(lightDir + viewDir);  
			float spec = material.z * pow(max(dot(norm, halfwayDir), 0.0), 32);

			float emission = material.w;

			// Final color
			vec3 result = (clamp(amb + diff + spec, 0.0, 1.0) * lightColor + emission) * objectColor;
			if (usesTexture == 1) {
				FragColor = texColor * vec4(result, 1.0) * eclipseFactor();
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

	shader.uniforms[LIGHT_POS] = glGetUniformLocation(shaderProgram, "lightPos");
	shader.uniforms[LIGHT_COLOR] = glGetUniformLocation(shaderProgram, "lightColor");
	shader.uniforms[LIGHT_RADIUS] = glGetUniformLocation(shaderProgram, "lightRadius");
	shader.uniforms[OBJ_COLOR] = glGetUniformLocation(shaderProgram, "objectColor");
	shader.uniforms[OBJ_MAT] = glGetUniformLocation(shaderProgram, "material");
	shader.uniforms[OBJ_OBLATE] = glGetUniformLocation(shaderProgram, "oblateness");
	shader.uniforms[VIEW_POS] = glGetUniformLocation(shaderProgram, "viewPos");
	shader.uniforms[OCCLUDERS] = glGetUniformLocation(shaderProgram, "occluders");
	shader.uniforms[NUM_OCCLUDERS] = glGetUniformLocation(shaderProgram, "numOccluders");
	shader.uniforms[TEX_BOOL] = glGetUniformLocation(shaderProgram, "usesTexture");
	shader.uniforms[NORM_BOOL] = glGetUniformLocation(shaderProgram, "usesNormalMap");
	shader.uniforms[TEX_MAP] = glGetUniformLocation(shaderProgram, "textureMap");
	shader.uniforms[NORMAL_MAP] = glGetUniformLocation(shaderProgram, "normalMap");

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
			gl_Position = vec4(pos.xyww);  // Keep the depth at the maximum value
		}  
	)";

	const char* fragmentSource = R"(
		#version 330 core
		in vec3 TexCoords;

		out vec4 FragColor;

		uniform samplerCube skybox;

		void main()
		{    
			FragColor = texture(skybox, TexCoords);
		}
	)";

	GLuint shaderProgram = compileShader(vertexSource, fragmentSource);

	Shader shader;
	glUseProgram(shaderProgram);
	shader.index = shaderProgram;
	shader.V = glGetUniformLocation(shaderProgram, "view");
	shader.P = glGetUniformLocation(shaderProgram, "projection");

	shader.uniforms[TEX_MAP] = glGetUniformLocation(shaderProgram, "skybox");

	return shader;
}

Shader initTrailShader() {
	const char* vertexSource = R"(
		#version 330 core
		layout (location = 0) in vec3 aPos;
		layout (location = 1) in float alpha;

		out float f_alpha;
		
		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

		void main() {
			gl_Position = projection * view * model * vec4(aPos, 1.0);
			float depth = 2.0 * log(1.0 + gl_Position.w) / log(1e9 + 1.0) - 1.0;
			gl_Position.z = depth * gl_Position.w;
			f_alpha = alpha;
		}
	)";

	const char* fragmentSource = R"(
		#version 330 core
		in float f_alpha;

		out vec4 fragColor;

		uniform vec3 color;

		void main() {
			fragColor = vec4(color, f_alpha); // Fade out trail with alpha
		}
	)";

	GLuint shaderProgram = compileShader(vertexSource, fragmentSource);

	Shader shader;
	glUseProgram(shaderProgram);
	shader.index = shaderProgram;
	shader.M = glGetUniformLocation(shaderProgram, "model");
	shader.V = glGetUniformLocation(shaderProgram, "view");
	shader.P = glGetUniformLocation(shaderProgram, "projection");
	shader.uniforms[OBJ_COLOR] = glGetUniformLocation(shaderProgram, "color");

	return shader;
}

Shader initFrameOverlayShader() {
	const char* vertexSource = R"(
		#version 330 core
		layout (location = 0) in vec2 aPos;
		layout (location = 1) in vec2 aTex;

		out vec2 TexCoords;
		
		uniform mat4 model;
		
		void main() {
			TexCoords = aTex;
			gl_Position = model * vec4(aPos.x, aPos.y, 0.0, 1.0); 
			float depth = 2.0 * log(1.0 + gl_Position.w) / log(1e9 + 1.0) - 1.0;
			gl_Position.z = depth * gl_Position.w;
		}  
	)";

	const char* fragmentSource = R"(
		#version 330 core
		in vec2 TexCoords;

		out vec4 FragColor;

		uniform sampler2D textureMap;

		void main() { 
			FragColor = texture(textureMap, TexCoords);
		}
	)";

	GLuint shaderProgram = compileShader(vertexSource, fragmentSource);

	Shader shader;
	glUseProgram(shaderProgram);
	shader.index = shaderProgram;
	shader.M = glGetUniformLocation(shaderProgram, "model");
	shader.uniforms[TEX_MAP] = glGetUniformLocation(shaderProgram, "textureMap");

	return shader;
}

Shader initSpriteShader() {
	const char* vertexSource = R"(
		#version 330 core

		layout (location = 0) in vec3 aPos;       // Quad vertex position (local space)
		layout (location = 1) in vec2 aTexCoord;  // Texture coordinates
		layout (location = 2) in vec3 instancePos; // Per-instance world position

		out vec2 TexCoord;

		uniform mat4 view;          // View matrix (without translation)
		uniform mat4 projection;
		uniform ivec2 windowSize;
		uniform ivec2 texSize;

		void main() {
			gl_Position = projection * view * vec4(instancePos, 1.0);

			TexCoord = aTexCoord;
    
			// scale the sprite based on window and texture dimensions
			vec2 ndcOffset = vec2(aPos.x * texSize.x / windowSize.x, aPos.y * texSize.y / windowSize.y);

			// Compute the final position of the sprite in clip space (considering its aspect ratio)
			gl_Position.xy += ndcOffset * gl_Position.w; // Scale correctly in perspective
			
			float depth = 2.0 * log(1.0 + gl_Position.w) / log(1e9 + 1.0) - 1.0;
			gl_Position.z = depth * gl_Position.w;
		}

	)";

	const char* fragmentSource = R"(
		#version 330 core

		in vec2 TexCoord;
		out vec4 FragColor;

		uniform sampler2D billboardTexture;

		void main() {
			FragColor = texture(billboardTexture, TexCoord);
			if (FragColor.a < 0.01)
				discard;
		}
	)";

	GLuint shaderProgram = compileShader(vertexSource, fragmentSource);

	Shader shader;
	glUseProgram(shaderProgram);
	shader.index = shaderProgram;
	shader.V = glGetUniformLocation(shaderProgram, "view");
	shader.P = glGetUniformLocation(shaderProgram, "projection");
	shader.uniforms[U_FOV] = glGetUniformLocation(shaderProgram, "aspect");
	shader.uniforms[WINDOW_SIZE] = glGetUniformLocation(shaderProgram, "windowSize");
	shader.uniforms[TEX_SIZE] = glGetUniformLocation(shaderProgram, "texSize");
	shader.uniforms[TEX_MAP] = glGetUniformLocation(shaderProgram, "billboardTexture");

	return shader;
}