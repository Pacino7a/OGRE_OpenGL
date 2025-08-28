#include <learnGL/glWithMethod.h>
#include <learnGL/camera.h>
#include <learnGL/shader.h>
#include <learnGL/model.h>
#include <learnGL/config.h>
#include <vector>

constexpr int MAJOR_VERSION{ 3 };
constexpr int MINOR_VERSION{ 3 };
constexpr int NR_POINT_LIGHTS{ 4 };
constexpr int SCR_WIDTH{ 800 };
constexpr int SCR_HEIGHT{ 600 };
constexpr bool MSAA{ true };
constexpr int SHADOW_WIDTH{ 1024 };
constexpr int SHADOW_HEIGHT{ 1024 };

auto g_lastX{ static_cast<double>(SCR_WIDTH) / 2 };
auto g_lastY{ static_cast<double>(SCR_HEIGHT) / 2 };
auto g_deltaTime{ 0.0f };
auto g_firstMoveMouse{ true };
auto g_framebuffer_width{ SCR_WIDTH };
auto g_framebuffer_height{ SCR_HEIGHT };

#ifdef __MULTISAMPLE__
auto MULTISAMPLER{ 4 };
#endif // __MULTISAMPLE__

Camera g_myCamera{ glm::vec3(0.0f, 0.0f, 3.0f) };

struct MatricesData
{
	glm::mat4 view;
	glm::mat4 projection;
	glm::vec4 viewPosPadding; // to align std140 as we expect, we need to make this a vec4 instead of vec3!
};


std::vector<glm::mat4> getShadowCubeMatrices(const glm::vec3& lightPos, float fov = 90.0f, float aspect = 1.0f, float near_plane = 0.1f, float far_plane = 25.0f)
{
	// Point light Matrices (6 dirs)
	glm::mat4 lightProjection = glm::perspective(glm::radians(fov), aspect, near_plane, far_plane); // to create a point shadow, we need a perspective LightSpace
	std::vector<glm::mat4> shadowTransforms;
	shadowTransforms.reserve(6);
	// Positive X
	shadowTransforms.emplace_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f,  -1.0f, 0.0f)));
	// Negative X
	shadowTransforms.emplace_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	// Positive Y
	shadowTransforms.emplace_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	// Negative Y
	shadowTransforms.emplace_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	// Positive Z
	shadowTransforms.emplace_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f,  -1.0f, 0.0f)));
	// Negative Z
	shadowTransforms.emplace_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

	return shadowTransforms;
}

std::vector <glm::vec3> calculateTangents(const glm::vec3& edge1, const glm::vec3& edge2,
										  const glm::vec2& deltaUV1, const glm::vec2& deltaUV2)
{
	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	glm::vec3 tangent1{};
	glm::vec3 bitangent1{};
	tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x); // deltaV2 and deltaV1
	tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

	bitangent1.x = f * (deltaUV1.x * edge2.x - deltaUV2.x * edge1.x); // deltaU1 and deltaU2
	bitangent1.y = f * (deltaUV1.x * edge2.y - deltaUV2.x * edge1.y);
	bitangent1.z = f * (deltaUV1.x * edge2.z - deltaUV2.x * edge1.z);
	
	return { tangent1, bitangent1 };
}

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

	Shader shader("vertexShaderFloor.vs", "fragShaderFloor.fs");
	Shader cubeShader("vertexShaderCube.vs", "fragShaderCube.fs");
	Shader depthShader("vertexShaderDepthMap.vs", "geometryDepth.gs", "fragShaderDepthMap.fs");
	//Shader scrShader("vertexShaderSCR.vs", "fragShaderSCR.fs");

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
		 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
		-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  0.0f,   10.0f,
		-10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  0.0f,   0.0f,

		 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
		 10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  10.0f,
		-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  0.0f,   10.0f
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

	/* How to create a TBN */
	// position coordinates (local space)
	glm::vec3 pos1{ -1.0, 1.0, 0.0 };
	glm::vec3 pos2{ -1.0,-1.0, 0.0 };
	glm::vec3 pos3{  1.0,-1.0, 0.0 };
	glm::vec3 pos4{  1.0, 1.0, 0.0 };
	// texture coordinates
	glm::vec2 uv1{ 0.0, 1.0 };
	glm::vec2 uv2{ 0.0, 0.0 };
	glm::vec2 uv3{ 1.0, 0.0 };
	glm::vec2 uv4{ 1.0, 1.0 };
	// (vertex) normal
	glm::vec3 normal{ 0.0, 0.0, 1.0 };
	// extract one of triangle of this plane
	glm::vec3 edge1{ pos2 - pos1 };
	glm::vec3 edge2{ pos3 - pos1 };
	glm::vec2 deltaUV1{ uv2 - uv1 }; // deltaU1(x) and deltaV1(y)
	glm::vec2 deltaUV2{ uv3 - uv1 }; // deltaU2 and deltaV2
	// formula procedure (functionlize)
	//float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
	//glm::vec3 tangent1{};
	//glm::vec3 bitangent1{};
	//tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	//tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	//tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	//bitangent1.x = f * (deltaUV1.x * edge2.x - deltaUV2.x * edge1.x);
	//bitangent1.y = f * (deltaUV1.x * edge2.y - deltaUV2.x * edge1.y);
	//bitangent1.z = f * (deltaUV1.x * edge2.z - deltaUV2.x * edge1.z);

	auto tangents1{ calculateTangents(edge1,edge2,deltaUV1,deltaUV2) };
	auto tangent1{ tangents1[0] };
	auto bitangent1{ tangents1[1] };

	// another one
	edge1 = pos4 - pos3;
	edge2 = pos1 - pos3;
	deltaUV1 = uv4 - uv3;
	deltaUV2 = uv1 - uv3;
	auto tangents2{ calculateTangents(edge1,edge2,deltaUV1,deltaUV2) };
	auto tangent2{ tangents2[0] };
	auto bitangent2{ tangents2[1] };

	// combine them as this plane's Tangent and Bitangent, we don't need do Normalize here.
	// because this operation will be implementated in Shaders
	// By the way, we only need to calculate the Tangents actually,
	// because we can get Bitangent by cross(Normal, Tangent).
	auto plane_tangent{ tangent1 + tangent2 };
	auto plane_bitangent{ bitangent1 + bitangent2 };

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
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4) + sizeof(glm::vec4), NULL, GL_STATIC_DRAW); // we don't have any dataSource assign to, we'll do this later
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	// Link
	/* for shader */
	unsigned int MatricesBlockIndex{ glGetUniformBlockIndex(shader.ID, "Matrices") }; // get Block Index in the Shader(GPU)
	glUniformBlockBinding(shader.ID, MatricesBlockIndex, 0); // GPUMem(shader) Links to Binding Point `0`
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO); // CPU(Buffer Obj) Links to Binding Point `0` (ALL Bind)
	unsigned int MatricesBlockIndex1{ glGetUniformBlockIndex(cubeShader.ID, "Matrices") }; // get Block Index in the Shader(GPU)
	glUniformBlockBinding(cubeShader.ID, MatricesBlockIndex1, 0); // GPUMem(shader) Links to Binding Point `0`
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO); // CPU(Buffer Obj) Links to Binding Point `0` (ALL Bind)

	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthMap); // a Point Light, need to use a CUBEMAP!
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
	/* A framebuffer object however is not complete without a color buffer
	  so we need to explicitly tell OpenGL we're not going to render any color data. */
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Framebuffer not complete!\n";
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// (Static) Point light Matrices (6 dirs)
	glm::vec3 lightPos{ 1.0f, 1.0f, 0.0f };
	float far_plane{ 25.0f };
	std::vector<glm::mat4> shadowTransforms{ };

	glm::mat4 model0{ glm::mat4(1.0f) };
	model0 = glm::translate(model0, glm::vec3(0.0f, -0.5f, 0.0f));
	//model = glm::scale(model, glm::vec3(2.0f)); // if you do this, (-0.5) will x2 -> Y-bias == -1.0 -> final Y == -1.5f
	glm::mat4 invModel{ glm::inverse(model0) };
	shader.use();
	shader.setMat4("model", model0);
	shader.setMat4("invModel", invModel);
	shader.setVec3("lightPos", lightPos);
	shader.setFloat("far_plane", far_plane);
	shader.setInt("texture1", 0);
	shader.setInt("shadowMap", 1);

	glm::mat4 model1{ glm::mat4(1.0f) };
	model1 = glm::translate(model1, glm::vec3(0.5f, -0.75f, 0.0f));
	model1 = glm::scale(model1, glm::vec3(0.5f));
	invModel = glm::inverse(model1);
	cubeShader.use();
	cubeShader.setMat4("model", model1);
	cubeShader.setMat4("invModel", invModel);
	cubeShader.setVec3("lightPos", lightPos);
	cubeShader.setFloat("far_plane", far_plane);
	cubeShader.setInt("texture1", 0);
	cubeShader.setInt("normalMap", 1);
	cubeShader.setInt("shadowMap", 2);

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

	unsigned int woodFloor{ loadTexture("D:/Cpp/textures/wood.jpg", GL_SRGB) };
	unsigned int cubeTex{ loadTexture("D:/Cpp/textures/brickwall.jpg", GL_SRGB) };
	unsigned int normalTex{ loadTexture("D:/Cpp/textures/brickwall_normal.jpg") };

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

		//glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//view = g_myCamera.getViewMatrix();
		//projection = glm::perspective(glm::radians(g_myCamera.getFov()), (float)g_width / g_height, 0.1f, 100.0f);
		//cameraPos = g_myCamera.getPosition();
		lightPos.x = static_cast<float>(sin(glfwGetTime() * 0.5) * 3.0);

		data.view = g_myCamera.getViewMatrix();
		data.projection = glm::perspective(glm::radians(g_myCamera.getFov()), (float)SCR_WIDTH/(float)SCR_HEIGHT, 0.1f, 100.0f);
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
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		depthShader.use();
		for (std::size_t i{ 0 }; i < 6; ++i)
		{
			depthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
		}
		depthShader.setVec3("lightPos", lightPos); // reset LightPos each Rendering Loop
		depthShader.setFloat("far_plane", far_plane);
		depthShader.setMat4("model", model0);
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		depthShader.setMat4("model", model1);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		/* rendering the scene with Shadows */
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.use();
		shader.setVec3("lightPos", lightPos); // vary Position
		glBindVertexArray(planeVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodFloor);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthMap);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		cubeShader.use();
		cubeShader.setVec3("lightPos", lightPos);
		glBindVertexArray(VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalTex);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthMap);
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
