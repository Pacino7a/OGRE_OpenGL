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

auto g_lastX{ static_cast<double>(g_width) / 2 };
auto g_lastY{ static_cast<double>(g_height) / 2 };
auto g_deltaTime{ 0.0f };
auto g_firstMoveMouse{ true };
auto g_framebuffer_width{ g_width };
auto g_framebuffer_height{ g_height };

Camera g_myCamera{ glm::vec3(0.0f, 0.0f, 3.0f) };

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
	Shader shaderWithGeo("vertexShaderGeo.vs", "geometryShader.gs", "fragShaderGeo.fs");
	Shader modelShader{ "vertexShaderModel.vs" ,"fragShaderModel.fs" };

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

	float points[]
	{
		-0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // top-left
		 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // top-right
		 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom-right
		-0.5f, -0.5f, 1.0f, 1.0f, 0.0f  // bottom-left
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
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glBindVertexArray(0);

	unsigned int pointVAO, pointVBO;
	glGenVertexArrays(1, &pointVAO);
	glGenBuffers(1, &pointVBO);
	glBindVertexArray(pointVAO);
	glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindVertexArray(0);

	/* Uniform Buffer Object */
	// Creation
	unsigned int UBO;
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, 2*sizeof(glm::mat4), NULL, GL_STATIC_DRAW); // we don't have any dataSource assign to, we'll do this later
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	// Link
	/* for shader */
	//unsigned int MatricesBlockIndex{ glGetUniformBlockIndex(shader.ID, "Matrices") }; // get Block Index in the Shader(GPU)
	//glUniformBlockBinding(shader.ID, MatricesBlockIndex, 0); // GPUMem(shader) Links to Binding Point `0`
	//glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO); // CPU(Buffer Obj) Links to Binding Point `0` (ALL Bind)
	//or 
	//glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, 2*sizeof(glm::mat4)); // selective bind
	/* for shader with Geometry Shader */
	unsigned int MatricesBlockIndex1{ glGetUniformBlockIndex(shaderWithGeo.ID, "Matrices") };
	glUniformBlockBinding(shaderWithGeo.ID, MatricesBlockIndex1, 1); // Binding Point `1`
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, UBO); // CPU(Buffer Obj) Links to Binding Point `1` (ALL Bind)
	/* for Model shader */
	unsigned int MatricesBlockIndex2{ glGetUniformBlockIndex(modelShader.ID, "Matrices") };
	glUniformBlockBinding(modelShader.ID, MatricesBlockIndex2, 2); // Binding Point `1`
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, UBO); // CPU(Buffer Obj) Links to Binding Point `1` (ALL Bind)
	// Notice that we binded UBO to 2 Binding Point right now

	//shader.use();
	//shader.setInt("texture1", 0);
	//model = glm::mat4(1.0f);
	//model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
	//model = glm::scale(model, glm::vec3(1.5f));
	//shader.setMat4("model", model);
	//glm::mat4 invModel{ glm::inverse(model) };
	//shader.setMat4("invModel", invModel);

	Model bag{ "D:/Cpp/resources/objects/backpack/backpack.obj" };

	shaderWithGeo.use();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
	model = glm::scale(model, glm::vec3(1.5f));
	shaderWithGeo.setMat4("model", model);
	shaderWithGeo.setMat4("invModel", glm::inverse(model));
	shaderWithGeo.setMat4("invModView", glm::inverse(view* model));
	modelShader.use();
	modelShader.setMat4("model", model);

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
		projection = glm::perspective(glm::radians(g_myCamera.getFov()), (float)g_width / g_height, 0.1f, 100.0f);

		// Assign Data to UBO -> Shader
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		//glBufferSubData(GL_UNIFORM_BUFFER, OFFSET to Beginning, Size of Input, Adress of Input data);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &view);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &projection);

		/* Simple test */
		//shader.use();
		//shader.setMat4("view", view);
		//shader.setMat4("projection", projection);
		//shader.setVec3("cameraPos", g_myCamera.getPosition());
		
		/* Draw Points and control their size */
		//glBindVertexArray(VAO);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, cubeTex);
		//glDrawArrays(GL_TRIANGLES, 0, 36);
		/* gl_PointSize */
		//glEnable(GL_PROGRAM_POINT_SIZE);
		//glDrawArrays(GL_POINTS, 0, 6);

		/* Use Geometry Shader */
		//shaderWithGeo.use();
		//glBindVertexArray(pointVAO);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//glDrawArrays(GL_POINTS, 0, 2);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//glDrawArrays(GL_POINTS, 2, 2);

		/* Expode Model */
		//shaderWithGeo.use();
		//shaderWithGeo.setFloat("time", glfwGetTime());
		//shaderWithGeo.setMat4("view", view);
		//shaderWithGeo.setMat4("projection", projection);
		//bag.drawModel(shaderWithGeo);

		/* Show Normals */
		bag.drawModel(modelShader); // Render the Model as Usual
		bag.drawModel(shaderWithGeo); // Render the Model by Geometry Shader (Transform Triangles into 3 Line-Strip)

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
