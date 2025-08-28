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

bool qePressed{ false };

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
		:	aPosition{pos}, aNormal{normal}, aTangent{tangent}, aTexcoords{texcoords}
	{ }
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
	Shader depthShader("vertexShaderDepthMap.vs", "geometryDepth.gs", "fragShaderDepthMap.fs");

	/* How to create a TBN */
	// position coordinates (local space)
	glm::vec3 pos1{ -1.0, 1.0, 0.0 };
	glm::vec3 pos2{ -1.0,-1.0, 0.0 };
	glm::vec3 pos3{ 1.0,-1.0, 0.0 };
	glm::vec3 pos4{ 1.0, 1.0, 0.0 };
	// texture coordinates
	glm::vec2 uv1{ 0.0, 1.0 };
	glm::vec2 uv2{ 0.0, 0.0 };
	glm::vec2 uv3{ 1.0, 0.0 };
	glm::vec2 uv4{ 1.0, 1.0 };
	// (vertex) normal
	glm::vec3 normal{ 0.0, 0.0, 1.0 };
	// extract one of triangle of this plane (1 2 3)
	glm::vec3 edge1{ pos2 - pos1 };
	glm::vec3 edge2{ pos3 - pos1 };
	glm::vec2 deltaUV1{ uv2 - uv1 }; // deltaU1(x) and deltaV1(y)
	glm::vec2 deltaUV2{ uv3 - uv1 }; // deltaU2 and deltaV2
	auto tangent1{ calculateTangents(edge1,edge2,deltaUV1,deltaUV2) };

	// another one (1, 3, 4)
	edge1 = pos4 - pos3;
	edge2 = pos1 - pos3;
	deltaUV1 = uv4 - uv3;
	deltaUV2 = uv1 - uv3;
	auto tangent2{ calculateTangents(edge1,edge2,deltaUV1,deltaUV2) };

	// combine them as this plane's Tangent and Bitangent, we don't need do Normalize here.
	// because this operation will be implementated in Shaders
	// By the way, we only need to calculate the Tangents actually,
	// because we can get Bitangent by cross(Normal, Tangent).
	auto plane_tangent{ tangent1 + tangent2 };

	/* Build Vertices */
	std::vector<MyVertex> wallVertices{};
	wallVertices.reserve(6); // one plane needs 6 vertices if you don't use glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0)
	wallVertices.emplace_back(MyVertex{ pos1, normal, plane_tangent, uv1 });
	wallVertices.emplace_back(MyVertex{ pos2, normal, plane_tangent, uv2 });
	wallVertices.emplace_back(MyVertex{ pos3, normal, plane_tangent, uv3 });
	wallVertices.emplace_back(MyVertex{ pos1, normal, plane_tangent, uv1 });
	wallVertices.emplace_back(MyVertex{ pos3, normal, plane_tangent, uv3 });
	wallVertices.emplace_back(MyVertex{ pos4, normal, plane_tangent, uv4 });

	auto wallVAO_VBO{ getVAO_VBO(wallVertices) };
	auto wallVAO{ wallVAO_VBO.first };
	auto wallVBO{ wallVAO_VBO.second };

	auto UBO{ getUBO(0, shader.ID, sizeof(MatricesData), "Matrices")}; // binding shader's Matrices Uniform Block to bindingPoint 0, so do CPU edge 

	auto depthCubeAndMap{ getDCMsFBO_MAP(SHADOW_WIDTH, SHADOW_HEIGHT) };
	auto depthCubeFBO{ depthCubeAndMap.first };
	auto depthCubeMap{ depthCubeAndMap.second };

	// (Static) Point light Matrices (6 dirs)
	glm::vec3 lightPos{ 1.0f, 1.0f, 1.0f };
	glm::mat4 model{ glm::mat4(1.0f) };
	float far_plane{ 25.0f };
	float height_scale{ 0.1f };
	std::vector<glm::mat4> shadowTransforms{ };

	shader.use();
	shader.setFloat("far_plane", far_plane);
	shader.setMat4("model", model);
	shader.setMat4("invModel", glm::inverse(model));
	shader.setInt("texture1", 0);
	shader.setInt("normalMap", 1);
	shader.setInt("shadowMap", 2);
	shader.setInt("displacementMap", 3);

	// rendering DepthCubeMap */ /* static Light Only*/
	//depthShader.use();
	//for (unsigned int i{ 0 }; i < 6; ++i)
	//{
	//	depthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
	//}
	//depthShader.setVec3("lightPos", lightPos);
	//depthShader.setFloat("far_plane", far_plane);
	//glEnable(GL_DEPTH_TEST);
	//glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	//glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	//glClear(GL_DEPTH_BUFFER_BIT);
	////depthShader.use();
	//depthShader.setMat4("model", model0);
	//glBindVertexArray(planeVAO);
	//glDrawArrays(GL_TRIANGLES, 0, 6);
	//depthShader.setMat4("model", model1);
	//glBindVertexArray(VAO);
	//glDrawArrays(GL_TRIANGLES, 0, 36);

	unsigned int wallTex{ loadTexture("D:/Cpp/textures/bricks2.jpg") };
	unsigned int wallNormalTex{ loadNormalTex("D:/Cpp/textures/bricks2_normal.jpg") };
	unsigned int wallDisplacement{ loadParallaxTex("D:/Cpp/textures/bricks_disp.jpg") };

	unsigned int toyTex{ loadTexture("D:/Cpp/textures/wood.png") };
	unsigned int toyNormalTex{ loadNormalTex("D:/Cpp/textures/toy_box_normal.png") };
	unsigned int toyParallax{ loadParallaxTex("D:/Cpp/textures/toy_box_disp.png") };

	MatricesData data;
	double currentFrame{};
	double lastFrame{};
	double lastSecond{};

	//glm::mat4 view{ glm::mat4(1.0f) };
	//glm::mat4 projection{ glm::mat4(1.0f) };
	//glm::vec3 cameraPos{};

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
		}

		processInput(window);

		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		{
			if (!qePressed && glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			{
				if (height_scale > 0.0f)
					height_scale -= 0.0005f;
				else
					height_scale = 0.0f;
			}
			else if (!qePressed && glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			{
				if (height_scale < 1.0f)
					height_scale += 0.0005f;
				else
					height_scale = 1.0f;
			}
			qePressed = true;
		}
		else
		{
			qePressed = false;
		}

		//view = g_myCamera.getViewMatrix();
		//projection = glm::perspective(glm::radians(g_myCamera.getFov()), (float)g_width / g_height, 0.1f, 100.0f);
		//cameraPos = g_myCamera.getPosition();
		lightPos.x = static_cast<float>(sin(glfwGetTime() * 0.5) * 3.0);

		data.view = g_myCamera.getViewMatrix();
		data.projection = glm::perspective(glm::radians(g_myCamera.getFov()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		data.viewPosPadding = glm::vec4(g_myCamera.getPosition(), 0.0f);

		// Assign Data to UBO -> Shader
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		/*glBufferSubData(GL_UNIFORM_BUFFER, OFFSET to Beginning, Size of Input, Adress of Input data);*/
		//glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
		//glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
		//glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::vec4), glm::value_ptr(cameraPos)); // vec3 -> align vec4
		/* All in */
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MatricesData), &data);

		/* Dynamic Point Light */
		/* rendering DepthCubeMap */ /* if the light's Position is Moving, we can render the depthMap in the loop */
		shadowTransforms = getShadowCubeMatrices(lightPos); // update LightMatrices

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthCubeFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		depthShader.use();
		for (std::size_t i{ 0 }; i < 6; ++i)
		{
			depthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
		}
		depthShader.setVec3("lightPos", lightPos); // reset LightPos each Rendering Loop
		depthShader.setFloat("far_plane", far_plane);
		/* renderScene() */
		glBindVertexArray(wallVAO);
		depthShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		/* rendering the scene with Shadows */
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.use();
		shader.setFloat("height_scale", height_scale);
		//std::cout << "Height Scale: " << height_scale << '\n';
		shader.setVec3("lightPos", lightPos); // vary Position
		glBindVertexArray(wallVAO);
		glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, wallTex);
		glBindTexture(GL_TEXTURE_2D, toyTex);

		glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, wallNormalTex);
		glBindTexture(GL_TEXTURE_2D, toyNormalTex);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);

		glActiveTexture(GL_TEXTURE3);
		//glBindTexture(GL_TEXTURE_2D, wallDisplacement);
		glBindTexture(GL_TEXTURE_2D, toyParallax);

		glDrawArrays(GL_TRIANGLES, 0, 6);


		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteFramebuffers(1, &depthCubeFBO);
	glDeleteVertexArrays(1, &wallVAO);
	glDeleteBuffers(1, &wallVBO);
	glDeleteBuffers(1, &UBO);

	glfwTerminate();

	return 0;
}