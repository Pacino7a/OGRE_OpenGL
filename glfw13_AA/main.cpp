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

auto g_lastX{ static_cast<double>(g_width) / 2 };
auto g_lastY{ static_cast<double>(g_height) / 2 };
auto g_deltaTime{ 0.0f };
auto g_firstMoveMouse{ true };
auto g_framebuffer_width{ g_width };
auto g_framebuffer_height{ g_height };
auto MULTISAMPLER{ 4 };

Camera g_myCamera{ glm::vec3(0.0f, 0.0f, 3.0f) };

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

	Shader shader("vertexShader.vs", "fragShader.fs");
	Shader scrShader("vertexShaderSCR.vs", "fragShaderSCR.fs");
	Shader customAAShader("vertexShaderSCR.vs", "fragShaderCustomSampler.fs");

	double currentFrame{};
	double lastFrame{};
	double lastSecond{};
	glm::mat4 model{ };
	glm::mat4 view{ glm::mat4(1.0f) };
	glm::mat4 projection{ glm::mat4(1.0f) };

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
	float quadVertices[]{   // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};
	unsigned int cubeTex{ loadTexture("D:/Cpp/textures/container2.png",GL_RGBA) };

	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(1);
	//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(2);
	//glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glBindVertexArray(0);

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

	/* MULTISAMPLED FRAMEBUFFER */
	unsigned int multisampledFBO;
	glGenFramebuffers(1, &multisampledFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, multisampledFBO);
	unsigned int tex; // multisample texture attachment
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MULTISAMPLER, GL_RGB, g_framebuffer_width, g_framebuffer_height, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex, 0);
	unsigned int RBO; // and multisample renderBuffer
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, MULTISAMPLER, GL_DEPTH24_STENCIL8, g_framebuffer_width, g_framebuffer_height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/* POST-EFFECT FRAMEBUFFER */ /* you can't sample a sampled FBO's (attached)texture, so you need transform it into a normal one*/
	unsigned int postFBO;
	glGenFramebuffers(1, &postFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, postFBO);
	unsigned int postTex; // normal texture attachment
	glGenTextures(1, &postTex);
	glBindTexture(GL_TEXTURE_2D, postTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_framebuffer_width, g_framebuffer_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postTex, 0);
	// no need depth or stencil anymore

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/* Uniform Buffer Object */
	// Creation
	unsigned int UBO;
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW); // we don't have any dataSource assign to, we'll do this later
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	// Link
	/* for shader */
	unsigned int MatricesBlockIndex{ glGetUniformBlockIndex(shader.ID, "Matrices") }; // get Block Index in the Shader(GPU)
	glUniformBlockBinding(shader.ID, MatricesBlockIndex, 0); // GPUMem(shader) Links to Binding Point `0`
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO); // CPU(Buffer Obj) Links to Binding Point `0` (ALL Bind)
	//or 
	//glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, 2*sizeof(glm::mat4)); // selective bind

	shader.use();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
	model = glm::scale(model, glm::vec3(1.5f));
	shader.setMat4("model", model);
	//glm::mat4 invModel{ glm::inverse(model) };
	//shader.setMat4("invModel", invModel);

	scrShader.use();
	scrShader.setInt("screenTexture", 0);

	customAAShader.use();
	customAAShader.setInt("screenTextureMS", 0);
	customAAShader.setInt("samples", MULTISAMPLER);
	customAAShader.setVec2i("viewportSize", g_width, g_height);
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	for (;!glfwWindowShouldClose(window);)
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
		}

		updateFramebuffer(tex, RBO, MSAA);
		processInput(window);

		glBindFramebuffer(GL_FRAMEBUFFER, multisampledFBO); // get Sampled Tex, NOT INCLUDE AA algorithm
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // for framebuffer msFBO
		glEnable(GL_DEPTH_TEST); // too

		view = g_myCamera.getViewMatrix();
		projection = glm::perspective(glm::radians(g_myCamera.getFov()), (float)g_width / g_height, 0.1f, 100.0f);

		// Assign Data to UBO -> Shader
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		//glBufferSubData(GL_UNIFORM_BUFFER, OFFSET to Beginning, Size of Input, Adress of Input data);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &view);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &projection);

		/* Simple Rendering test */
		shader.use();
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		/* Blite multisampled framebuffer */ /* if you want to Custom your own AA, commented these statements (DON'T GL Blite)*/
		//glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampledFBO);
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // MSAA FBO -> Default FB -> out (if you write this, you don't need statements below)
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postFBO); // sampled texture -> normal texture
		//glBlitFramebuffer(0, 0, g_framebuffer_width, g_framebuffer_height, 0, 0, g_framebuffer_width, g_framebuffer_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		//* Render quad with scene's visuals as its texture image */ /* Custom needs this */
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT); // clear framebuffer 0 for rendering
		//glDisable(GL_DEPTH_TEST); // because we make the scene a Texture (2D Image), so we don't need This anymore. We only need to print it out

		//* draw Screen quad */ /* MSAA FBO -> PostEffect FBO -> Default FB -> scrShader */
		//scrShader.use();
		//glBindVertexArray(quadVAO);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, postTex); // we can create some POST EFFECT right now!
		//glDrawArrays(GL_TRIANGLES, 0, 6);

		/* Custom AA */ /* Pass MSTex into this shader and print it on the Screen */ /* if you want to Do some Post Effects,*/
		/* Do Post Effects (you need to 3 blocks above to use custom AA) */
		/* MS FBO(get Sampled Tex) -> customAA FBO(do AA algorithm) -> postFBO(MSAA Tex -> 2D Tex) */
		glBindFramebuffer(GL_FRAMEBUFFER, postFBO);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT); // clear framebuffer 0 for rendering
		glDisable(GL_DEPTH_TEST);

		/* if you want to Directly Output to Framebuffer0, Please block Blite and draw SCR above */
		/* and just keep this Alone */
		customAAShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex); // tex is MSTex
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		/* Then Print the result with Post Effects on the Screen as usual*/
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		
		scrShader.use();
		glBindVertexArray(quadVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, postTex); // we can create some POST EFFECT right now!
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glfwTerminate();

	return 0;
}
