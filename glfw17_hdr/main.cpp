#include <learnGL/glWithMethod.h>
#include <learnGL/camera.h>
#include <learnGL/shader.h>
#include <learnGL/config.h>
#include <vector>

constexpr int MAJOR_VERSION{ 3 };
constexpr int MINOR_VERSION{ 3 };
constexpr int NR_POINT_LIGHTS{ 4 };
constexpr int SCR_WIDTH{ 800 };
constexpr int SCR_HEIGHT{ 600 };
constexpr int SHADOW_WIDTH{ 1024 };
constexpr int SHADOW_HEIGHT{ 1024 };
constexpr bool MSAA{ true };

auto g_lastX{ static_cast<double>(SCR_WIDTH) / 2 };
auto g_lastY{ static_cast<double>(SCR_HEIGHT) / 2 };
auto g_deltaTime{ 0.0f };
auto g_firstMoveMouse{ true };
auto g_framebuffer_width{ SCR_WIDTH };
auto g_framebuffer_height{ SCR_HEIGHT };

#ifdef __MULTISAMPLE__
auto MULTISAMPLER{ 8 };
#endif // __MULTISAMPLE__

Camera g_myCamera{ glm::vec3(0.0f, 0.0f, 3.0f) };


struct MatricesData
{
	glm::mat4 view;
	glm::mat4 projection;
	glm::vec4 viewPosPadding; // to align std140 as we expect, we need to make this a vec4 instead of vec3!
};

struct MyVertex
{
	glm::vec3 aPosition;
	glm::vec3 aNormal;
	glm::vec3 aTangent;
	glm::vec2 aTexcoords;

	MyVertex(const glm::vec3& pos, const glm::vec3& normal, const glm::vec3& tangent, const glm::vec2& texcoords)
		: aPosition{ pos }, aNormal{ normal }, aTangent{ tangent }, aTexcoords{ texcoords }
	{
	}
};

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MAJOR_VERSION);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MINOR_VERSION);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	/* MSAA */
	glfwWindowHint(GLFW_SAMPLES, MULTISAMPLER);


	GLFWwindow* window{ glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL) };
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
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallBack);
	glfwSetCursorPosCallback(window, cursorCallbackClass); // to register the callback funtions we need a GLOBAL CAMERA
	glfwSetScrollCallback(window, scrollCallbackClass);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	Shader shader("vertexShader.vs", "fragShader.fs");
	Shader HDRShader("vertexShaderHDRQuad.vs", "fragShaderHDRQuad.fs");
	Shader blurShader("vertexShaderHDRQuad.vs", "fragShaderGaussian.fs");
	Shader lightShader("vertexShaderLight.vs", "fragShaderLight.fs");

	// Uniform Block Obj
	auto UBO{ getUBO(0, shader.ID, sizeof(MatricesData), "Matrices") }; // binding shader's Matrices Uniform Block to bindingPoint 0, so do CPU edge 
	// add binding Shader of this UBO
	unsigned int MatricesBlockIndex1{ glGetUniformBlockIndex(lightShader.ID, "Matrices") }; // get Block Index in the Shader(GPU)
	glUniformBlockBinding(lightShader.ID, MatricesBlockIndex1, 0); // GPUMem(shader) Links to Binding Point `0`
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO); // CPU(Buffer Obj) Links to Binding Point `0` (ALL Bind)
	// Single FBO with One HDR texture
	auto hdrFTR{ getHDR_FBO_Tex() }; // returns {fbo, tex, rbo}
	auto hdrFBO{ hdrFTR[0]};
	auto hdrTex{ hdrFTR[1]};
	// Single FBO with HDR color texture and BrightTexture
	std::vector<unsigned int> hdrTextures(2); // you need to initialize a textures container first!
	auto hdrFR_m{ getHDR_FBO_MutipleTex(hdrTextures) }; // returns {fbo, rbo}
	auto hdrFBO_m{ hdrFR_m[0] };
	auto hdrColorBuffer{ hdrTextures[0] }; // extract the container
	auto brightColorBuffer{ hdrTextures[1] };
	// Ping-Pong FBO for cross Rendering Gaussian Blur
	auto pingpongFBOs_Buffers{ getPingPongFBOs() }; // returns {fbo0, fbo1, colorbuffer0, colorbuffer1}
	std::vector pingpongFBOs{ pingpongFBOs_Buffers[0], pingpongFBOs_Buffers[1] };
	std::vector pingpongBuffers{ pingpongFBOs_Buffers[2], pingpongFBOs_Buffers[3]}; // pass one (horizontal Blur), pass two (vertical Blur)


	glm::mat4 model{ glm::mat4(1.0f) };
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 25.0));
	model = glm::scale(model, glm::vec3(2.5f, 2.5f, 27.5f));

	shader.use();
	shader.setMat4("model", model);
	shader.setMat4("invModel", glm::inverse(model));
	shader.setBool("invNormal", true);
	shader.setInt("texture1", 0);

	// lighting info
	// -------------
	// positions
	std::vector<glm::vec3> lightPositions;
	lightPositions.emplace_back(glm::vec3(0.0f, 0.0f, 49.5f)); // back light
	lightPositions.emplace_back(glm::vec3(-1.4f, -1.9f, 9.0f));
	lightPositions.emplace_back(glm::vec3(0.0f, -1.8f, 4.0f));
	lightPositions.emplace_back(glm::vec3(0.8f, -1.7f, 6.0f));
	// colors
	std::vector<glm::vec3> lightColors;
	lightColors.emplace_back(glm::vec3(200.0f, 200.0f, 200.0f));
	lightColors.emplace_back(glm::vec3(0.1f, 0.0f, 0.0f));
	lightColors.emplace_back(glm::vec3(0.0f, 0.0f, 0.2f));
	lightColors.emplace_back(glm::vec3(0.0f, 0.1f, 0.0f));

	for (unsigned int i = 0; i < lightPositions.size(); ++i)
	{
		shader.setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
		shader.setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
	}

	bool hdrEnable{ true };
	float exposure{ 1.0f };
	HDRShader.use();
	HDRShader.setBool("hdrEnable", hdrEnable);
	HDRShader.setFloat("exposure", exposure);
	HDRShader.setInt("hdrTex", 0);
	HDRShader.setInt("bloomBlur", 1);

	blurShader.use();
	blurShader.setInt("image", 0);

	MatricesData data;
	double currentFrame{};
	double lastFrame{};
	double lastSecond{};
	int amount{ 10 };

	unsigned int cubeVAO{};
	unsigned int cubeVBO{};
	unsigned int quadVAO{};
	unsigned int quadVBO{};

	unsigned int wood{ loadTexture("D:/Cpp/textures/wood.png") };

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE); // framebuffer0 automatically Do AA when rendering something into it
	// but if you use FrameBuffer0 to receive the scene's texture, it will not apply MSAA if you didn't use multisample textures

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
		}

		processInput(window);

		data.view = g_myCamera.getViewMatrix();
		data.projection = glm::perspective(glm::radians(g_myCamera.getFov()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		data.viewPosPadding = glm::vec4(g_myCamera.getPosition(), 0.0f);

		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MatricesData), &data); // (target, offset, size, data)

		// render Scene to HDR framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO_m);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wood);
		renderCube(cubeVAO, cubeVBO);

		/* Lights Rendering */
		float angle = (float)glfwGetTime() * 25.0f;
		lightShader.use();
		for (std::size_t i{ 1 }; i < lightPositions.size(); ++i)
		{
			model = glm::mat4(1.0f);
			// Orbiting
			//model = glm::translate(model, lightPositions[i]);
			//model = glm::rotate(model, glm::radians(angle), glm::vec3{ 0.0f, 1.0f, 0.0f });
			//model = glm::translate(model, glm::vec3{ 0.0f, 0.0f, 0.3f }); // translate after Rotate is The key of Orbiting
			// Self-rotation
			model = glm::translate(model, lightPositions[i]);
			model = glm::rotate(model, glm::radians(angle), glm::vec3{ 0.0f, 1.0f, 0.0f }); // rotate by self, NO TRANSLATE LATER!
			model = glm::scale(model, glm::vec3(0.3f));
			lightShader.setMat4("model", model);
			lightShader.setVec3("lightColor", lightColors[i]);
			renderCube(cubeVAO, cubeVBO);
		}

		/* Gaussian Blur the Scene's Bright Part (texturelized now) */
		bool horizontal = true;
		bool first_iteration = true;
		blurShader.use();
		glActiveTexture(GL_TEXTURE0);
		for (unsigned int i{ 0 }; i < amount; ++i)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBOs[horizontal]);
			blurShader.setInt("horizontal", horizontal);
			glBindTexture(
				GL_TEXTURE_2D, first_iteration ? brightColorBuffer : pingpongBuffers[!horizontal]
			);
			renderQuad(quadVAO, quadVBO);
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		HDRShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffers[1]);
		renderQuad(quadVAO, quadVBO);

		//std::cout << "HDR: " << (hdrEnable ? "ON" : "OFF") << '\n';
		//std::cout << "Exposure: " << exposure << '\n';

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteFramebuffers(1, &hdrFBO);
	glDeleteFramebuffers(1, &hdrFBO_m);
	glDeleteFramebuffers(2, pingpongFBOs.data());
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &quadVBO);
	glDeleteBuffers(1, &UBO);

	glfwTerminate();

	return 0;
}