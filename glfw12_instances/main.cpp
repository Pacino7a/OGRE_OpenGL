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

Camera g_myCamera{ glm::vec3(0.0f, 15.0f, 200.0f) };

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MAJOR_VERSION);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MINOR_VERSION);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Anti-Aliasings MSAA*/
	glfwWindowHint(GLFW_SAMPLES, 4);

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
	Shader shaderM("vertexShaderM.vs", "fragShaderM.fs");

	Shader shaderPlant("vertexShaderPlanet.vs", "fragShaderPlanet.fs");
	Shader shaderRock("vertexShaderRock.vs", "fragShaderRock.fs");

	Model planet{ "D:/Cpp/resources/objects/planet/planet.obj" };
	Model rock{ "D:/Cpp/resources/objects/rock/rock.obj" };

	double currentFrame{};
	double lastFrame{};
	double lastSecond{};
	glm::mat4 model{ };
	glm::mat4 view{ glm::mat4(1.0f) };
	glm::mat4 projection{ glm::mat4(1.0f) };

	float quadVertices[] = {
		// positions     // colors
		-0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
		 0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
		-0.05f, -0.05f,  0.0f, 0.0f, 1.0f,

		-0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
		 0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
		 0.05f,  0.05f,  0.0f, 1.0f, 1.0f
	};

	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindVertexArray(0);

	/* Uniform Buffer Object */
	// Creation
	unsigned int UBO;
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW); // we don't have any dataSource assign to, we'll do this later
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	// Link
	/* for shader */
	unsigned int matricesBlockIndex{ glGetUniformBlockIndex(shaderPlant.ID, "Matrices") }; // get Block Index in the Shader(GPU)
	glUniformBlockBinding(shaderPlant.ID, matricesBlockIndex, 0); // GPUMem(shader) Links to Binding Point `0`
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO); // CPU(Buffer Obj) Links to Binding Point `0` (ALL Bind)
	unsigned int matricesBlockIndex1{ glGetUniformBlockIndex(shaderRock.ID, "Matrices") };
	glUniformBlockBinding(shaderRock.ID, matricesBlockIndex1, 1);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, UBO);
	unsigned int matricesBlockIndex2{ glGetUniformBlockIndex(shaderM.ID, "Matrices") };
	glUniformBlockBinding(shaderM.ID, matricesBlockIndex2, 2);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, UBO);

	//shaderPlant.use();
	//model = glm::mat4(1.0f);
	//model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
	//model = glm::scale(model, glm::vec3(0.5f));
	//shaderPlant.setMat4("model", model);
	//auto invModel{ glm::inverse(model) };
	//shader.setMat4("invModel", invModel);

	//shaderRock.use();
	//auto modelR{ glm::mat4(1.0f) };
	//modelR = glm::translate(modelR, glm::vec3(3.0f, -0.5f, 0.0f));
	//modelR = glm::scale(modelR, glm::vec3(0.5f));
	//shaderRock.setMat4("model", modelR);

	//std::vector<glm::vec2> translations{}; // Top left Coordinate of the window is(-1, 1), Bottom right is (1, 1)
	//translations.reserve(100);
	//constexpr float offset{ 0.1f }; // move to bottom right
	//float x{ -1.0f };
	//float y{  1.0f };
	//for (size_t i = 1; i <= 100; ++i)
	//{
	//	translations.emplace_back(glm::vec2{ x + offset, y - offset });
	//	x += 0.2f;
	//	if (i % 10 == 0)
	//	{
	//		x = -1.0f;
	//		y -= 0.2f;
	//	}
	//}

	/*	When we are rendering a lot more than 100 instances (which is quite common)
		we will eventually hit a limit on the amount of uniform data we can send to the shaders.*/

	//shader.use(); // because uniform's Limitation is 100, so we need change our way
	//for (std::size_t i{ 0 }; i < 100; ++i)
	//{
	//	shader.setVec2("offset[" + std::to_string(i) + "]", translations[i]);
	//}

	/* Asteroid Ring ModelMatrices Create */
	unsigned int amount{ 100000 };
	//glm::mat4* modelMatrices;
	//modelMatrices = new glm::mat4[amount];
	std::vector<glm::mat4> modelMatrices{};
	modelMatrices.resize(amount);
	srand(glfwGetTime()); // initialize random seed	
	float radius = 150.0f;
	float offset = 25.0f;
	for (unsigned int i = 0; i < amount; i++)
	{
		model = glm::mat4(1.0f);
		// 1. translation: displace along circle with 'radius' in range [-offset, offset]
		float angle = (float)i / (float)amount * 360.0f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;
		model = glm::translate(model, glm::vec3(x, y, z));

		// 2. scale: scale between 0.05 and 0.25f
		float scale = (rand() % 20) / 100.0f + 0.05;
		model = glm::scale(model, glm::vec3(scale));

		// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
		float rotAngle = (rand() % 360);
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		// 4. now add to list of matrices
		modelMatrices[i] = model;
	}

	/* Set the Matrices as Vertex Attribute */
	auto& rockMeshes{ rock.getMeshes() };
	constexpr std::size_t vec4Size{ sizeof(glm::vec4) };

	unsigned int modelMatricesVBO;
	glGenBuffers(1, &modelMatricesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, modelMatricesVBO);
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::vec4), modelMatrices.data(), GL_STATIC_DRAW);

	unsigned int meshVAO;
	for (unsigned int i = 0; i < rockMeshes.size(); ++i)
	{
		meshVAO = rockMeshes[i].getMeshVAO();
		glBindVertexArray(meshVAO);
		// vertex attributes (Mat4 is 4 Vec4)
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
		glEnableVertexAttribArray(8);
		glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
		glEnableVertexAttribArray(9);
		glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
		glEnableVertexAttribArray(10);
		glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

		glVertexAttribDivisor(7, 1);
		glVertexAttribDivisor(8, 1);
		glVertexAttribDivisor(9, 1);
		glVertexAttribDivisor(10, 1);

		glBindVertexArray(0);
	}

	/* For simple instances test */
	//unsigned int offsetVBO;
	//glGenBuffers(1, &offsetVBO);
	//glBindVertexArray(VAO);
	//glBindBuffer(GL_ARRAY_BUFFER, offsetVBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 100, &translations[0], GL_STATIC_DRAW);
	//glEnableVertexAttribArray(4);
	//glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	/* Key of instance */
	//glVertexAttribDivisor(4, 1); // the first represents attribute index, the second is freq of Updating it
	//							 // 0 means update every Vertex, `1 means updating every instance`, n(>1) means updating every n instances
	//glBindVertexArray(0);

	/* Pass through Model Matrices As UNIFORMs */
	//unsigned int matricesVBO;
	//glGenBuffers(1, &matricesVBO);
	//glBindVertexArray(VAO);
	//glBindBuffer(GL_ARRAY_BUFFER, matricesVBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * amount, modelMatrices.data(), GL_STATIC_DRAW);
	//glEnableVertexAttribArray(4);
	//glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	//glVertexAttribDivisor(4, 1);
	//glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

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
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		view = g_myCamera.getViewMatrix();
		projection = glm::perspective(glm::radians(g_myCamera.getFov()), (float)g_width / g_height, 0.1f, 1000.0f);

		/* Assign Data to UBO -> Shader */
		/* glBufferSubData(GL_UNIFORM_BUFFER, OFFSET to Beginning, Size of Input, Adress of Input data) */
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &view);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &projection);

		/* Simple test */
		//shader.use();
		//shader.setMat4("view", view);
		//shader.setMat4("projection", projection);
		//shader.setVec3("cameraPos", g_myCamera.getPosition()); // for lights or reflections, refractions

		/* Instances Test */
		//shader.use();
		//glBindVertexArray(VAO);
		  // glDrawArrays(GL_TRIANGLES, 0, 6);
		//glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);

		/* Planet & Rock Test */
		// 1 Planet Rendering Call
		shaderM.use();
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		shaderM.setMat4("model", model);
		shaderM.setBool("notInsDraw", true);
		planet.drawModel(shaderM);
		shaderM.setBool("notInsDraw", false);
		//rock.drawModel(shaderRock);
		
		// 1000 Asteriod Ring's Rocks Renderings call (When the number is big, this will be Performance Costy)
		//for (unsigned int i = 0; i < amount; i++)
		//{
		//	shaderM.setMat4("model", modelMatrices[i]);
		//	rock.drawModel(shaderM);
		//}

		rock.drawModel(shaderM, amount);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &meshVAO);

	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &modelMatricesVBO);

	glfwTerminate();
}
