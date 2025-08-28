#include <learnGL/glWithMethod.h>
#include <learnGL/camera.h>
#include <learnGL/shader.h>
#include <vector>

constexpr int MAJOR_VERSION{ 3 };
constexpr int MINOR_VERSION{ 3 };
constexpr int NR_POINT_LIGHTS{ 4 };
constexpr int g_width{ 800 };
constexpr int g_height{ 600 };

auto g_lastX{ static_cast<double>(g_width) / 2 };
auto g_lastY{ static_cast<double>(g_height) / 2 };
auto g_deltaTime{ 0.0f };
auto g_firstMoveMouse{ true };
auto g_framebuffer_width{ g_width };
auto g_framebuffer_height{ g_height };

Camera g_myCamera{ glm::vec3(0.0f, 0.0f, 3.0f) };

std::vector<std::string> getCubeMapAbsPaths(const std::string& basePath, const std::vector<std::string>& fileNames)
{
	std::vector<std::string> paths{};
	paths.reserve(4);
	for (const auto& name : fileNames)
	{
		paths.emplace_back(basePath + name);
	}
	return paths;
}
int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MAJOR_VERSION);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MINOR_VERSION);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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
	Shader skyShader("vertexShaderSky.vs", "fragShaderSky.fs");


	double currentFrame{};
	double lastFrame{};
	double lastSecond{};
	glm::mat4 model{ };
	glm::mat4 viewSky{ glm::mat4(1.0f) };
	glm::mat4 view{ glm::mat4(1.0f) };
	glm::mat4 projection{ glm::mat4(1.0f) };

	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

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

	// IDs Control the resources, work with glBindxx()
	std::string basePath{ "D:/Cpp/textures/skybox/" };
	std::vector<std::string> fileNames{ 
		"right.jpg",
		"left.jpg",
		"top.jpg",
		"bottom.jpg",
		"front.jpg",
		"back.jpg"
	};
	auto paths{ getCubeMapAbsPaths(basePath, fileNames) };
	unsigned int skybox{ loadCubeMapTextures(paths) }; // load Skybox's cubeMap
	
	unsigned int cubeTex{ loadTexture("D:/Cpp/textures/container2.png",GL_RGBA) };

	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glBindVertexArray(0);

	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glBindVertexArray(0);

	skyShader.use();
	skyShader.setInt("skybox", 0);
	shader.use();
	shader.setInt("texture1", 0);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
	model = glm::scale(model, glm::vec3(1.5f));
	shader.setMat4("model", model);
	glm::mat4 invModel{ glm::inverse(model) };
	shader.setMat4("invModel", invModel);
	shader.setInt("skybox", 1);

	glEnable(GL_DEPTH_TEST);

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
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		view = g_myCamera.getViewMatrix();
		viewSky = glm::mat4(glm::mat3(g_myCamera.getViewMatrix()));  // remove the shift
		projection = glm::perspective(g_myCamera.getFov(), (float)g_width / g_height, 0.1f, 100.f);
		
		shader.use();
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		shader.setVec3("cameraPos", g_myCamera.getPosition());

		glBindVertexArray(VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glEnable(GL_PROGRAM_POINT_SIZE);
		glDrawArrays(GL_POINTS, 0, 6);

		skyShader.use();
		skyShader.setMat4("view", viewSky);
		skyShader.setMat4("projection", projection);
		
		//glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL); // because the depth value of the skybox is always be 1.0f,
								// so if there is a enetity front of it(LESS situation), the skybox's fragments will be discarded
								// otherwise, the blank places' depth value is 1.0f, so at the EQUAL situation will render the skybox. 
								// In the other words, the skybox will only be rendered on the blank places of the screen for optimization.
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
