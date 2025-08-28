#include <learnGL/glWithMethod.h>
#include <learnGL/model.h>
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
	//glfwWindowHint(GLFW_SAMPLES, MULTISAMPLER);


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

	bool normalize{ true };
	Model backPack{ "D:/Cpp/resources/objects/backpack/backpack.obj" };
	Model sphere{ "D:/Cpp/resources/objects/sphere/sphere.obj", normalize};

	Shader shaderGeometryPass("vertexShaderGpass.vs", "fragShaderGpass.fs");
	Shader shaderLightingPass("vertexShaderLightpass.vs", "fragShaderLightpass.fs");
	Shader shaderLight("vertexShaderLight.vs", "fragShaderLight.fs");
	Shader shaderAmbient("vertexShaderHDRQuad.vs", "fragShaderAmbient.fs");
	Shader shaderQuad("vertexShaderHDRQuad.vs", "fragShaderHDRQuad.fs");
	Shader shaderSSAO("vertexShaderSSAO.vs", "fragShaderSSAO.fs");
	Shader shaderSSAOBlur("vertexShaderSSAO.vs", "fragShaderSSAOBlur.fs");
	Shader shaderFloor("vertexShaderFloor.vs", "fragShaderFloor.fs");

	// Light source Shader needs Uniform Block
	auto UBO{ getUBO(0, shaderLight.ID, sizeof(MatricesData), "Matrices") };
	// so do Geometry Pass Shader
	unsigned int MatricesBlockIndex1{ glGetUniformBlockIndex(shaderGeometryPass.ID, "Matrices") };
	glUniformBlockBinding(shaderGeometryPass.ID, MatricesBlockIndex1, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);
	unsigned int MatricesBlockIndex2{ glGetUniformBlockIndex(shaderFloor.ID, "Matrices") };
	glUniformBlockBinding(shaderFloor.ID, MatricesBlockIndex2, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);

	// Gbuffer FBO and its attachments
	auto gBufferAttribities{ getGbuffer() }; // return { gBuffer, gPosition, gNormal, gAlbedoSpec, RBOdepth };
	auto gbuffer{ gBufferAttribities[0] };
	auto gPositionTex{ gBufferAttribities[1] };
	auto gNormalTex{ gBufferAttribities[2] };
	auto gAlbedoSpec{ gBufferAttribities[3] };
	auto gRBOdepth{ gBufferAttribities[4] };

	auto hdrFBO_Tex{ getHDR_FBO_Tex() };
	auto hdrFBO{ hdrFBO_Tex[0] };
	auto hdrTex{ hdrFBO_Tex[1] };

	// SSAO FBO and Color Buffer
	auto ssaoFBO_Buffer{ getSSAO_FBO_Tex() }; // return { ssaoFBO, ssaoColorBuffer }
	auto ssaoFBO{ ssaoFBO_Buffer[0] };
	auto ssaoColorBuffer{ ssaoFBO_Buffer[1] };

	// SSAO Blur Buffer
	auto ssaoBlurFBO_Buffer{ getSSAO_FBO_Tex() };
	auto ssaoBlurFBO{ ssaoBlurFBO_Buffer[0] };
	auto ssaoBlurColor{ ssaoBlurFBO_Buffer[1] };

	// SSAO Kernel and Noise
	auto ssaoKernel{ getSSAOKernel() };
	auto ssaoKernelSize{ ssaoKernel.size() };
	auto ssaoNoise{ getSSAONoise() };
	auto ssaoNoiseTex{ getSSAONoiseTex(ssaoNoise) };

	// lighting info
	// -------------
	const unsigned int NR_LIGHTS = 12;
	std::vector<glm::vec3> lightPositions;
	std::vector<glm::vec3> lightColors;
	srand(13);
	for (unsigned int i = 0; i < NR_LIGHTS; ++i)
	{
		// calculate slightly random offsetsz
		float xPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
		float yPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 4.0);
		float zPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
		lightPositions.push_back(glm::vec3(xPos, yPos, zPos));
		// also calculate random color
		float rColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
		float gColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
		float bColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
		lightColors.push_back(glm::vec3(rColor, gColor, bColor));
	}

	glm::vec3 lightPos = glm::vec3(2.0, 4.0, -2.0);
	glm::vec3 lightColor = glm::vec3(0.2, 0.2, 0.7);
	//glm::vec3 lightColor = glm::vec3(1.0, 1.0, 1.0);


	const float constant = 1.0;
	const float linear = 0.7f;
	const float quadratic = 1.8f;
	float lightMax{};
	float radius{};

	shaderLightingPass.use();
	shaderLightingPass.setInt("gPosition", 0);
	shaderLightingPass.setInt("gNormal", 1);
	shaderLightingPass.setInt("gAlbedoSpec", 2);
	shaderLightingPass.setInt("ssao", 3);
	//shaderLightingPass.setVec3("light.Position", lightPos); // in world Space! we don't need this
	shaderLightingPass.setVec3("light.Color", lightColor);
	shaderLightingPass.setFloat("light.Linear", linear);
	shaderLightingPass.setFloat("lights.Quadratic", quadratic);
	for (std::size_t i = 0; i < lightPositions.size(); ++i)
	{
		shaderLightingPass.setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
		shaderLightingPass.setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
		// update attenuation parameters and calculate radius
		shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Linear", linear);
		shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Quadratic", quadratic);

		lightMax = std::fmaxf(std::fmaxf(lightColors[i].r, lightColors[i].g), lightColors[i].b);
		radius =
			(-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0 / 5.0) * lightMax)))
			/ (2 * quadratic);
		shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Radius", radius);

	}

	shaderAmbient.use();
	shaderAmbient.setInt("gAlbedoSpec", 0);
	shaderAmbient.setVec3("ambientColor", glm::vec3(0.1f, 0.1f, 0.1f));

	shaderQuad.use();
	shaderQuad.setFloat("exposure", 1.0f);
	shaderQuad.setInt("hdrTex", 0);

	MatricesData data;
	double currentFrame{};
	double lastFrame{};
	double lastSecond{};

	unsigned int cubeVAO{};
	unsigned int cubeVBO{};
	unsigned int quadVAO{};
	unsigned int quadVBO{};
	unsigned int floorVAO{};
	unsigned int floorVBO{};

	glm::mat4 model{};

	shaderFloor.use();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0, -3.0f, 0.0f));
	shaderFloor.setMat4("model", model);
	shaderFloor.setMat4("invModel", glm::inverse(model));
	shaderFloor.setInt("texture1", 0);
	shaderFloor.setVec3("lightPos", lightPositions[0]);

	shaderSSAO.use();
	shaderSSAO.setInt("gPosition", 0);
	shaderSSAO.setInt("gNormal", 1);
	shaderSSAO.setInt("texNoise", 2);

	shaderSSAOBlur.use();
	shaderSSAOBlur.setInt("ssaoInput", 0);

	auto wood{ loadTexture("D:/Cpp/textures/wood.jpg") };

	auto lightPosView{ lightPos };

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_MULTISAMPLE); // framebuffer0 automatically Do AA when rendering something into it
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

		/* Geometry pass*/ // Scene to Texture
		glBindFramebuffer(GL_FRAMEBUFFER, gbuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//shaderFloor.use();
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, woodFloor);
		//renderFloor(floorVAO, floorVBO);
		shaderGeometryPass.use();
		// room cube
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0, 7.0f, 0.0f));
		model = glm::scale(model, glm::vec3(7.5f, 7.5f, 7.5f));
		shaderGeometryPass.setMat4("model", model);
		shaderGeometryPass.setBool("invertedNormals", true); // invert normals as we're inside the cube
		shaderGeometryPass.setInt("material.texture_diffuse1", 0); // cube is our temporal model, so we need load texture and specify specular
		shaderGeometryPass.setBool("manualSpec", true); // set Specular Intensity Manually
		shaderGeometryPass.setFloat("specular", 0.25f);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wood);
		renderCube(cubeVAO, cubeVBO);
		shaderGeometryPass.setBool("manualSpec", false); // set with specular_Texture
		shaderGeometryPass.setBool("invertedNormals", false);
		// backpack model on the floor
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
		model = glm::scale(model, glm::vec3(1.0f));
		shaderGeometryPass.setMat4("model", model);
		backPack.drawModel(shaderGeometryPass);

		/* Compute SSAO Value */
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		glClear(GL_COLOR_BUFFER_BIT);
		shaderSSAO.use();
		// Send kernel + rotation 
		for (unsigned int i = 0; i < ssaoKernelSize; ++i)
			shaderSSAO.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
		shaderSSAO.setMat4("projection", data.projection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPositionTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormalTex);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, ssaoNoiseTex); // set random rotation
		renderQuad(quadVAO, quadVBO);

		/* Blur SSAO result */
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
		glClear(GL_COLOR_BUFFER_BIT);
		shaderSSAOBlur.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		renderQuad(quadVAO, quadVBO);

		/* LightPass */
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderLightingPass.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPositionTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormalTex);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, ssaoBlurColor);
		lightPosView = glm::vec3(data.view * glm::vec4(lightPos, 1.0f));
		shaderLightingPass.setVec3("light.Position", lightPosView); // notice we pass in LightPos in View Space
		renderQuad(quadVAO, quadVBO);

		/* Print Scene */
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderQuad.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdrTex);
		renderQuad(quadVAO, quadVBO);

		/* add Light Sources to The Result */ // Render Lights To the Scene Texture
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gbuffer); // copy content of geometry's depth buffer to default framebuffer's depth buffer
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		shaderLight.use();
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.125f));
		shaderLight.setMat4("model", model);
		shaderLight.setVec3("lightColor", lightColor);
		sphere.drawModel(shaderLight);
		//for (unsigned int i = 0; i < lightPositions.size(); ++i)
		//{
		//	model = glm::mat4(1.0f);
		//	model = glm::translate(model, lightPositions[i]);
		//	model = glm::scale(model, glm::vec3(0.125f));
		//	shaderLight.setMat4("model", model);
		//	shaderLight.setVec3("lightColor", lightColors[i]);
		//	//renderCube(cubeVAO, cubeVBO);
		//	sphere.drawModel(shaderLight);
		//}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteFramebuffers(1, &gbuffer);
	glDeleteFramebuffers(1, &ssaoFBO);
	glDeleteFramebuffers(1, &ssaoBlurFBO);
	glDeleteFramebuffers(1, &hdrFBO);
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &quadVBO);
	glDeleteBuffers(1, &UBO);

	glfwTerminate();

	return 0;
}