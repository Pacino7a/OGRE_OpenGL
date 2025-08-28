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
	{}
};

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MAJOR_VERSION);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MINOR_VERSION);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	/* MSAA */ // when you use G-Buffer, please turn OFF MSAA!
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
	Model sphere{ "D:/Cpp/resources/objects/sphere/sphere.obj", normalize };
	Model gun{ "D:/Cpp/resources/objects/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX" };
	
	/* Sphere */
	//auto albedo{ loadTexture("D:/Cpp/textures/rustIronPBR/rustediron2_basecolor.png", GL_SRGB_ALPHA) };
	//auto metallic{ loadParallaxTex("D:/Cpp/textures/rustIronPBR/rustediron2_metallic.png") }; // not parallax texture, just the function only reads one channel data
	//auto normalTex{ loadNormalTex("D:/Cpp/textures/rustIronPBR/rustediron2_normal.png") };
	//auto roughness{ loadParallaxTex("D:/Cpp/textures/rustIronPBR/rustediron2_roughness.png") };

	/* Gun */ /* Notice the texture has been reversed */
	auto albedo{ loadTexture("D:/Cpp/textures/GUN/Cerberus_A.jpg", false) }; // the texture is stored from left-bottom conner
	auto metallic{ loadParallaxTex("D:/Cpp/textures/GUN/Cerberus_M.jpg", false) }; // not parallax texture, just the function only reads one channel data
	auto normalTex{ loadNormalTex("D:/Cpp/textures/GUN/Cerberus_N.jpg", false) };
	auto roughness{ loadParallaxTex("D:/Cpp/textures/GUN/Cerberus_R.jpg", false) };

	auto hdrEquirectangular{ loadHDRadiance("D:/Cpp/textures/HDR_029_Sky_Cloudy_Free/HDR_029_Sky_Cloudy_Ref.hdr") };

	Shader shaderPBR("vertexShaderPBR.vs", "fragShaderPBR.fs"); // to use Textures, you only need switch fragShader to PBRt
	Shader shaderPBR_Model("vertexShaderPBR_Model.vs", "fragShaderPBR_Model.fs");
	Shader shaderLight("vertexShaderLight.vs", "fragShaderLight.fs");
	Shader shaderEq2Cube("vertexShaderEq2Cube.vs", "fragShaderEq2Cube.fs");
	Shader shaderSkybox("vertexShaderSky.vs", "fragShaderSky.fs");
	Shader shaderIrradiance("vertexShaderSky.vs", "fragShaderRadiance.fs");
	Shader shaderPrefilter("vertexShaderEq2Cube.vs", "fragShaderPrefilter.fs");
	Shader shaderBRDF("vertexShaderBRDFconv.vs", "fragShaderBRDFconv.fs");
	Shader shaderSCR("vertexShaderSCR.vs", "fragShaderSCR.fs");

	// Light source Shader needs Uniform Block
	auto UBO{ getUBO(0, shaderPBR.ID, sizeof(MatricesData), "Matrices") };
	// so do Geometry Pass Shader
	unsigned int MatricesBlockIndex1{ glGetUniformBlockIndex(shaderPBR.ID, "Matrices") };
	glUniformBlockBinding(shaderPBR.ID, MatricesBlockIndex1, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);
	unsigned int MatricesBlockIndex2{ glGetUniformBlockIndex(shaderLight.ID, "Matrices") };
	glUniformBlockBinding(shaderLight.ID, MatricesBlockIndex2, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);
	unsigned int MatricesBlockIndex3{ glGetUniformBlockIndex(shaderPBR_Model.ID, "Matrices") };
	glUniformBlockBinding(shaderPBR_Model.ID, MatricesBlockIndex3, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);

	// Gbuffer FBO and its attachments
	//auto gBufferAttribities{ getGbuffer() }; // return { gBuffer, gPosition, gNormal, gAlbedoSpec, RBOdepth };
	//auto gbuffer{ gBufferAttribities[0] };
	//auto gPositionTex{ gBufferAttribities[1] };
	//auto gNormalTex{ gBufferAttribities[2] };
	//auto gAlbedoSpec{ gBufferAttribities[3] };
	//auto gRBOdepth{ gBufferAttribities[4] };

	//auto hdrFBO_Tex{ getHDR_FBO_Tex() };
	//auto hdrFBO{ hdrFBO_Tex[0] };
	//auto hdrTex{ hdrFBO_Tex[1] };

	// lighting info
	// -------------
	glm::vec3 lightPositions[] = {
		glm::vec3(-10.0f,  10.0f, 10.0f),
		glm::vec3(10.0f,  10.0f, 10.0f),
		glm::vec3(-10.0f, -10.0f, 10.0f),
		glm::vec3(10.0f, -10.0f, 10.0f),
	};
	glm::vec3 lightColors[] = {
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f)
	};

	//glm::vec3 lightPositions[] = {
	//glm::vec3(0.0f, 0.0f, 10.0f),
	//};
	//glm::vec3 lightColors[] = {
	//	glm::vec3(150.0f, 150.0f, 150.0f),
	//};

	/* preparations for HDR cubeMap transformation */
	auto captureProjection{ glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f) };
	std::vector captureViews{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f,  -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f,  -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f,  -1.0f,  0.0f))
	};
	const auto capturesFBO_Cube{ getCaptureFBO_envCubeMap() };
	const auto captureFBO{ capturesFBO_Cube[0] };
	const auto captureCubeMap{ capturesFBO_Cube[1] };
	const auto captureRBO{ capturesFBO_Cube[2] };
	unsigned int cubeVAO{};
	unsigned int cubeVBO{};

	/* equirectangular environmemt map to cubemap */
	shaderEq2Cube.use();
	shaderEq2Cube.setInt("equirectangularMap", 0); // texture0 has been occupied
	shaderEq2Cube.setMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrEquirectangular);

	glViewport(0, 0, 512, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (int i{ 0 }; i < 6; ++i)
	{
		shaderEq2Cube.setMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, captureCubeMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderCube(cubeVAO, cubeVBO); // render requirectangular to <cubeMap>
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glBindTexture(GL_TEXTURE_CUBE_MAP, captureCubeMap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);


	/* Albedo - Diffuse PART */
	/* finally, we need transform the cubeMap to Irradiance cubeMap for sampling */
	/* pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.*/
	unsigned int irradianceCubeMap;
	glGenTextures(1, &irradianceCubeMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceCubeMap);
	for (int i{ 0 }; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
			32, 32, 0, GL_RGB, GL_FLOAT, nullptr); // allocate space
		// a LOW resolution for irradiance map is fine, linear filter will do most of the work for us
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* re-scale the CaptureBuffer to the NEW resolution */
	//glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	/* render Irradiance result (cubeMap -> irradiance cubeMap) */
	shaderIrradiance.use();
	shaderIrradiance.setInt("environmentMap", 0); // texture0 has been occupied
	shaderIrradiance.setMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, captureCubeMap);

	glViewport(0, 0, 32, 32);
	/*glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);*/

	/* Rendering IrradianceMap from HDR CubeMap */
	/* if you want to render the Irradiance to attachment 1, but IT'S NO necessary! */
	//GLenum drawBuffer = GL_COLOR_ATTACHMENT1;
	//glDrawBuffers(1, &drawBuffer);

	for (int i{ 0 }; i < 6; ++i) 
	{
		shaderIrradiance.setMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceCubeMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderCube(cubeVAO, cubeVBO); // render requirectangular to cubeMap
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	/* Specular PART */
	unsigned int prefilteredMap;
	glGenTextures(1, &prefilteredMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilteredMap);
	for (int i{ 0 }; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr); // if you have Something which has more smooth materials(like Cars), you can use a high resolution Specular Resolution
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // we have Mipmap
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	/* Get PrefilterMap and its Mipmap */
	shaderPrefilter.use();
	shaderPrefilter.setInt("environmentMap", 0);
	shaderPrefilter.setMat4("projection", captureProjection);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, captureCubeMap);

	const unsigned int maxMipmapLevel{ 5 };
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	for (unsigned int level{ 0 }; level < maxMipmapLevel; ++level)
	{
		unsigned int mipMapWidth{ static_cast<unsigned int>(128 * std::pow(0.5, level)) };
		unsigned int mipMapHeight{ static_cast<unsigned int>(128 * std::pow(0.5, level)) };
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipMapWidth, mipMapHeight);
		glViewport(0, 0, mipMapWidth, mipMapHeight);

		float roughness{ (float)level / (float)maxMipmapLevel };
		shaderPrefilter.setFloat("roughness", roughness);
		for (std::size_t i{ 0 }; i < 6; ++i)
		{
			shaderPrefilter.setMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilteredMap, level);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderCube(cubeVAO, cubeVBO);
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// a high roughness will create seams between cubeMap faces
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // Simply enable this property somewhere at the start of your application and the seams will be gone.
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	/* Render BRDF PreRendering-Texture */ // BRDF don't need any MAP, we just saved a LUT in a TEXTURE
	unsigned int brdfTexture;				// which input is just TexCoords -> (A,B) -> Texture.rg for (NdotV, roughness) looking up
	glGenTextures(1, &brdfTexture);
	glBindTexture(GL_TEXTURE_2D, brdfTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfTexture, 0);
	glViewport(0, 0, 512, 512);

	unsigned int quadVAO{}; // VAOs and VBOs must be (zero)initialized!
	unsigned int quadVBO{};
	shaderBRDF.use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderQuad(quadVAO, quadVBO);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	/* BRDF's result Test */
	shaderSCR.use();
	shaderSCR.setInt("scrMap", 0);

	/* Show CubeMap */
	shaderSkybox.use();
	shaderSkybox.setInt("environmentMap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, captureCubeMap);

	glm::mat4 model{};

	shaderPBR_Model.use();
	//shaderPBR.setVec3("albedo", 0.5f, 0.0f, 0.0f);
	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians( 90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.05f));
	shaderPBR_Model.setMat4("model", model);
	shaderPBR_Model.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
	shaderPBR_Model.setFloat("ao", 1.0f);
	shaderPBR_Model.setInt("albedoMap", 0);
	shaderPBR_Model.setInt("normalMap", 1);
	shaderPBR_Model.setInt("metallicMap", 2);
	shaderPBR_Model.setInt("roughnessMap", 3);
	//shaderPBR.setInt("aoMap", 4);
	shaderPBR_Model.setInt("irradianceMap", 5);
	shaderPBR_Model.setInt("prefilterMap", 6);
	shaderPBR_Model.setInt("brdfLUT", 7);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, albedo);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, metallic);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, roughness);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceCubeMap);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilteredMap);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, brdfTexture);

	/* useful variables for rendering loop */
	MatricesData data;
	double currentFrame{};
	double lastFrame{};
	double lastSecond{};
	int nrRows{ 7 };
	int nrColumns{ 7 };
	float spacing{ 2.5 };

	unsigned int sphereVAO{};
	unsigned int sphereIndexCount{};

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	//glEnable(GL_MULTISAMPLE); // framebuffer0 automatically Do AA when rendering something into it
	// but if you use FrameBuffer0 to receive the scene's texture, it will not apply MSAA until you use multisample textures

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

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* render Background */
		shaderSkybox.use();
		shaderSkybox.setMat4("view", data.view);
		shaderSkybox.setMat4("projection", data.projection);
		renderCube(cubeVAO, cubeVBO);

		// render light Sources
		shaderPBR_Model.use();
		for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
		{
			glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
			newPos = lightPositions[i];
			shaderPBR_Model.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
			shaderPBR_Model.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);

			shaderLight.use();
			model = glm::mat4(1.0f);
			model = glm::translate(model, newPos);
			model = glm::scale(model, glm::vec3(0.5f));
			shaderLight.setMat4("model", model);
			shaderLight.setVec3("lightColor", lightColors[i]);
			//sphere.drawModel();
			renderSphere(sphereVAO, sphereIndexCount);
			shaderPBR_Model.use();
		}

		/* Metallic and Roughness Test */
		//for (int row{ 0 }; row < nrRows; ++row)
		//{
		//	shaderPBR.setFloat("metallic", (float)row / (float)nrRows);
		//	for (int col{ 0 }; col < nrColumns; ++col)
		//	{
		//		shaderPBR.setFloat("roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f));
		//		model = glm::mat4(1.0f);
		//		model = glm::translate(model,
		//			glm::vec3((col - (nrColumns / 2)) * spacing, (row - (nrRows / 2)) * spacing, 0.0f));
		//		shaderPBR.setMat4("model", model);
		//		shaderPBR.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
		//		//sphere.drawModel(shaderPBR);
		//		renderSphere(sphereVAO, sphereIndexCount);
		//	}
		//}

		//shaderPBR_Model.use();
		//renderSphere(sphereVAO, sphereIndexCount);
		
		gun.drawModel_Native(shaderPBR_Model); // require Material Definition in shaders


		//shaderSCR.use();
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, brdfTexture);
		//renderQuad(quadVAO, quadVBO);
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &sphereVAO);
	glDeleteBuffers(1, &UBO);

	glfwTerminate();

	return 0;
}