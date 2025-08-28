#include <learnGL/glWithMethod.h>
#include <learnGL/camera.h>
#include <learnGL/shader.h>
#include <learnGL/model.h>
#include <vector>

constexpr int MAJOR_VERSION{ 3 };
constexpr int MINOR_VERSION{ 3 };
constexpr int NR_POINT_LIGHTS{ 4 };
constexpr int g_width{ 800 };
constexpr int g_height{ 600 };
constexpr bool MSAA{ true };
constexpr int SHADOW_WIDTH{ 1024 };
constexpr int SHADOW_HEIGHT{ 1024 };

auto g_lastX{ static_cast<double>(g_width) / 2 };
auto g_lastY{ static_cast<double>(g_height) / 2 };
auto g_deltaTime{ 0.0f };
auto g_firstMoveMouse{ true };
auto g_framebuffer_width{ g_width };
auto g_framebuffer_height{ g_height };

auto MULTISAMPLER{ 4 };
auto g_blinn{ true };
auto g_blinnKeyPressed{ false };
auto g_gammaEnable{ true };
auto g_gammaKeyPressed{ false };

Camera g_myCamera{ glm::vec3(0.0f, 0.0f, 3.0f) };

struct MatricesData
{
	glm::mat4 view;
	glm::mat4 projection;
	glm::vec4 viewPosPadding;
	int blinn;
	int gamma;
};


int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MAJOR_VERSION);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MINOR_VERSION);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	/* MSAA */
	glfwWindowHint(GLFW_SAMPLES, MULTISAMPLER);


	GLFWwindow* window{ glfwCreateWindow(g_width, g_height, "LearnOpenGL", NULL, NULL) };
	if (!window)
	{
		std::cerr << "Sorry, we failed to create a GLFW window.\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Sorry, we failed to initialize GLAD.\n";
		return -1;
	}
	glViewport(0, 0, g_width, g_height);

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallBack);
	glfwSetCursorPosCallback(window, cursorCallbackClass); // to register the callback funtions we need a GLOBAL CAMERA
	glfwSetScrollCallback(window, scrollCallbackClass);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	Shader shader("vertexShaderFloor.vs", "fragShaderFloor.fs");
	Shader cubeShader("vertexShaderCube.vs", "fragShaderCube.fs");
	Shader depthShader("vertexShaderDepthMap.vs", "fragShaderDepthMap.fs");
	Shader scrShader("vertexShaderSCR.vs", "fragShaderSCR.fs");

	double currentFrame{};
	double lastFrame{};
	double lastSecond{};
	glm::mat4 model0{ };
	glm::mat4 model1{ };
	glm::mat4 view{ glm::mat4(1.0f) };
	glm::mat4 projection{ glm::mat4(1.0f) };
	glm::vec3 cameraPos{};

	float cubeVertices[] = {
		// Back face
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,  // Bottom-left
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,  // top-right
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  // bottom-right
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,  // top-right
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,  // bottom-left
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 0.0f, -1.0f,  // top-left
		// Front face
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, 0.0f,  1.0f,  // bottom-left
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 0.0f,  1.0f,  // bottom-right
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f, 0.0f,  1.0f,  // top-right
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f, 0.0f,  1.0f,  // top-right
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  0.0f, 0.0f,  1.0f,  // top-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, 0.0f,  1.0f,  // bottom-left
		// Left face
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,  // top-right
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  // top-left
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  // bottom-left
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  // bottom-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,  // bottom-right
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,  // top-right
		// Right face
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // top-left
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f,  // bottom-right
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  1.0f, 0.0f, 0.0f,  // top-right
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f,  // bottom-right
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // top-left
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom-left
		 // Bottom face
		 -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f, // top-right
		  0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  0.0f, -1.0f, 0.0f, // top-left
		  0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f, // bottom-left
		  0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f, // bottom-left
		 -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, -1.0f, 0.0f, // bottom-right
		 -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f, // top-right
		 // Top face
		 -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f, // top-left
		  0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		  0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f, 1.0f, 0.0f, // top-right     
		  0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		 -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f, // top-left
		 -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  0.0f, 1.0f, 0.0f  // bottom-left        
	};

	float planeVertices[] = {
		// positions            // normals         // texcoords
		 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   10.0f,  0.0f,
		-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f,   10.0f,
		-10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,   0.0f,

		 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   10.0f,  0.0f,
		 10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   10.0f,  10.0f,
		-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f,   10.0f
	};

	float quadVertices[]{   // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glBindVertexArray(0);

	// plane VAO
	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	// Quad VAO
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	/* Uniform Buffer Object */
	// Creation
	unsigned int UBO;
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4) + sizeof(glm::vec4) + 8, NULL, GL_STATIC_DRAW); // we don't have any dataSource assign to, we'll do this later
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Link
	/* for shader */
	unsigned int MatricesBlockIndex{ glGetUniformBlockIndex(shader.ID, "Matrices") }; // get Block Index in the Shader(GPU)
	glUniformBlockBinding(shader.ID, MatricesBlockIndex, 0); // GPUMem(shader) Links to Binding Point `0`
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO); // CPU(Buffer Obj) Links to Binding Point `0` (ALL Bind)
	//or 
	//glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, 2*sizeof(glm::mat4)); // selective bind
	unsigned int MatricesBlockIndex1{ glGetUniformBlockIndex(cubeShader.ID, "Matrices") }; // get Block Index in the Shader(GPU)
	glUniformBlockBinding(cubeShader.ID, MatricesBlockIndex1, 0); // GPUMem(shader) Links to Binding Point `0`
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO); // CPU(Buffer Obj) Links to Binding Point `0` (ALL Bind)
	
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] { 1.0f, 1.0f, 1.0f, 1.0f }; // 白色表示无阴影
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	/* A framebuffer object however is not complete without a color buffer
	  so we need to explicitly tell OpenGL we're not going to render any color data. */
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Framebuffer not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glm::vec3 lightPos(-2.0f, 2.0f, -1.0f);

	model0 = glm::mat4(1.0f);
	model0 = glm::translate(model0, glm::vec3(0.0f, -0.5f, 0.0f));
	//model = glm::scale(model, glm::vec3(2.0f)); // if you do this, (-0.5) will x2 -> Y-bias == -1.0 -> final Y == -1.5f
	glm::mat4 invModel{ glm::inverse(model0) };
	shader.use();
	shader.setMat4("model", model0);
	shader.setMat4("invModel", invModel);
	shader.setVec3("lightPos", lightPos);
	shader.setInt("texture1", 0);
	shader.setInt("shadowMap", 1);

	model1 = glm::mat4(1.0f);
	model1 = glm::scale(model1, glm::vec3(0.5f)); // bottom's y == (-0.5 * 0.5 )-0.25
	model1 = glm::translate(model1, glm::vec3(1.0f, -1.5f, 0.0f)); // -1.5 * 0.5 == -0.75 -> bottom's y == -1.0f
	invModel = glm::inverse(model1);
	cubeShader.use();
	cubeShader.setMat4("model", model1);
	cubeShader.setMat4("invModel", invModel);
	cubeShader.setVec3("lightPos", lightPos);
	cubeShader.setInt("texture1", 0);
	cubeShader.setInt("shadowMap", 1);

	unsigned int woodFloor{ loadTexture("D:/Cpp/textures/wood.jpg", GL_SRGB) };
	unsigned int cubeTex{ loadTexture("D:/Cpp/textures/container2.png", GL_SRGB_ALPHA) };

	MatricesData data;

	cubeShader.use();

	scrShader.use();
	scrShader.setInt("depthMap", 0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE); // framebuffer0 automatically Do AA when rendering something into it

	for (; !glfwWindowShouldClose(window);)
	{
		currentFrame = glfwGetTime(); // currentFrameTime
		g_deltaTime = static_cast<float>(currentFrame - lastFrame); // time elapsed in the last frame
		lastFrame = currentFrame; // save current time for the next calculation

		if (currentFrame - lastSecond > 1)
		{
			lastSecond = currentFrame;
			auto fps{ 1 / g_deltaTime };
			std::ostringstream strStream;
			strStream << "LearnOpenGL FPS:" << fps << "";
			glfwSetWindowTitle(window, strStream.str().c_str());

			std::cout << "Specualr Model:" << (g_blinn ? " Blinn" : " Phong") << '\n'
					  << "Gamma Correction: " << (g_gammaEnable ? "Enable" : "Disable") << '\n';
		}

		processInput(window);
		if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) // Falling edge
		{
			if (!g_blinnKeyPressed)
			{
				g_blinn = !g_blinn;
				g_blinnKeyPressed = true;
			}
		}
		else
		{
			g_blinnKeyPressed = false;
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			if (!g_gammaKeyPressed)
			{
				g_gammaEnable = !g_gammaEnable;
				g_gammaKeyPressed = true;
			}
		}
		else
		{
			g_gammaKeyPressed = false;
		}

		//glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//view = g_myCamera.getViewMatrix();
		//projection = glm::perspective(glm::radians(g_myCamera.getFov()), (float)g_width / g_height, 0.1f, 100.0f);
		//cameraPos = g_myCamera.getPosition();
		//int blinn_int = g_blinn ? 1 : 0;
		//int gamma_int = g_gammaEnable ? 1 : 0;

		data.view = g_myCamera.getViewMatrix();
		data.projection = glm::perspective(glm::radians(g_myCamera.getFov()), (float)g_width / g_height, 0.1f, 100.0f);
		data.viewPosPadding = glm::vec4(g_myCamera.getPosition(), 0.0f);
		data.blinn = g_blinn ? 1 : 0;
		data.gamma = g_gammaEnable ? 1 : 0;

		// Assign Data to UBO -> Shader
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		/*glBufferSubData(GL_UNIFORM_BUFFER, OFFSET to Beginning, Size of Input, Adress of Input data);*/
		//glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
		//glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
		//glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::vec4), glm::value_ptr(cameraPos)); // vec3 -> align vec4
		//glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4) + sizeof(glm::vec4), sizeof(int), &blinn_int);
		//glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4) + sizeof(glm::vec4) + sizeof(int), sizeof(int), &gamma_int);
		/* All in */
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MatricesData), &data);
		
		// directional light Matrix
		float near_plane = 1.0f, far_plane = 8.0f;
		glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		//glm::mat4 lightView = glm::lookAt(lightPos,
		//	lightPos + glm::vec3(0.0f, -1.0f, 0.0f), // pos + front
		//	glm::vec3(0.0f, 0.0f, 1.0f)); // the camera(light)'s up should not be parallel with its direction (-front)
		glm::mat4 lightView = glm::lookAt(lightPos,
			glm::vec3(0.0f, 0.0f, 0.0f), // pos + front (target)
			glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		/* rendering DepthMap */
		glDisable(GL_CULL_FACE); // Don't cull floor
		depthShader.use();
		depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		depthShader.setMat4("model", model0);
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		// glEnable(GL_CULL_FACE); // only Fix solid objects' acne (BUT sometimes the shadow on the floor not real)
		glCullFace(GL_FRONT);
		depthShader.setMat4("model", model1);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glCullFace(GL_BACK); // back to original

		/* rendering the scene */
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, g_width, g_height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		/* Shadow Map Virtualization */
		//scrShader.use();
		//glBindVertexArray(quadVAO);
		//glBindTexture(GL_TEXTURE_2D, depthMap);
		//glDrawArrays(GL_TRIANGLES, 0, 6);

		/* rendering the scene with Shadows */
		shader.use();
		shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		glBindVertexArray(planeVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodFloor);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		cubeShader.use();
		cubeShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		glBindVertexArray(VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glDrawArrays(GL_TRIANGLES, 0, 36);


		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteFramebuffers(1, &depthMapFBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);
	glDeleteBuffers(1, &UBO);
	
	glfwTerminate();

	return 0;
}
