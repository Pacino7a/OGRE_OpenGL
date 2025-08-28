#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include<learnGL/shader.h>
#include<learnGL/stb_image.h>
#include <learnGL/camera.h>

constexpr int MAJOR_VERSION{ 3 };
constexpr int MINOR_VERSION{ 3 };
constexpr int g_width{ 800 };
constexpr int g_height{ 600 };

auto g_lastX{ static_cast<double>(g_width) / 2 };
auto g_lastY{ static_cast<double>(g_height) / 2 };
auto g_deltaTime{ 0.0f };
bool g_firstMoveMouse{ true };

Camera g_myCamera{ glm::vec3(0.0f, 0.0f, 3.0f) };

void framebufferSizeCallBack(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // if the function captured ESC input, we close the window.
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		g_myCamera.processKeyboard(Data::forward, g_deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		g_myCamera.processKeyboard(Data::backward, g_deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		g_myCamera.processKeyboard(Data::left, g_deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		g_myCamera.processKeyboard(Data::right, g_deltaTime);
}

void cursorCallbackClass(GLFWwindow* window, double xpos, double ypos)
{
	if (g_firstMoveMouse)
	{
		g_lastX = xpos;
		g_lastY = ypos;
		g_firstMoveMouse = false;
	}
	// calculate the offset between current frame and the last frame
	float xOffset{ static_cast<float>(xpos - g_lastX) };
	float yOffset{ static_cast<float>(ypos - g_lastY) };
	// save current position for calculating the next bias
	g_lastX = xpos;
	g_lastY = ypos;

	g_myCamera.processMouseMove(xOffset, yOffset);
}

void scrollCallbackClass(GLFWwindow* window, double xOffset, double yOffset)
{
	g_myCamera.processMouseScroll(yOffset);
}

unsigned int loadTexture(const char* path, GLenum format = GL_RGB)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//float borderColor[]{ 0.0f,0.0f,0.5f,1.0f };
	//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nClrChannel;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data{ stbi_load(path, &width, &height, &nClrChannel, 0) };
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format == GL_RGBA ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "We FAILED to load Texture!\n";
	}
	stbi_image_free(data);
	return textureID;
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MAJOR_VERSION);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MINOR_VERSION);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window{ glfwCreateWindow(g_width,g_height,"LearnOpenGL",NULL,NULL) };
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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, cursorCallbackClass); // to register the callback funtions we need a GLOBAL CAMERA
	glfwSetScrollCallback(window, scrollCallbackClass);

	float vertices[] = {
		// 6 vertices in each surface
		// 36 vertices in total to build ONE cube
		// x, y, z			  Tex x,y
		//-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		// 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		// 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		// 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		//-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		//-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		//-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		// 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		// 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		// 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		//-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		//-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		//-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		//-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		//-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		//-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		//-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		//-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		// 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		// 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		// 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		// 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		// 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		// 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		//-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		// 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		// 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		// 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		//-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		//-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		//-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		// 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		// 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		// 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		//-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		//-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	
	// coordinates,			// normal
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};

	glm::vec3 cubePositions[] = {
	glm::vec3(0.0f,  0.0f,  0.0f),
	glm::vec3(2.0f,  5.0f, -15.0f),
	glm::vec3(-1.5f, -2.2f, -2.5f),
	glm::vec3(-3.8f, -2.0f, -12.3f),
	glm::vec3(2.4f, -0.4f, -3.5f),
	glm::vec3(-1.7f,  3.0f, -7.5f),
	glm::vec3(1.3f, -2.0f, -2.5f),
	glm::vec3(1.5f,  2.0f, -2.5f),
	glm::vec3(1.5f,  0.2f, -1.5f),
	glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	// Light Source Shader
	Shader lightShader{ "vertexShader_lightSource.vs", "fragShader_lightSource.fs" };
	// Entity Shader
	Shader shader{ "vertexShader.vs", "fragShader_Materials.fs" };
	Shader shaderView{ "vertexShader_lightVIEW.vs", "fragShader_lightVIEW.fs" };
	Shader shaderGouraud{ "vertexShader_Gouraud.vs", "fragShader_Gouraud.fs" };


	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // read normal data
	glEnableVertexAttribArray(3); // activate normal attribute

	/* WE DONT BIND TEXTURES FOR NOW */
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
	//glEnableVertexAttribArray(1);
	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(2);

	//unsigned int texture1{ loadTexture("D:/Cpp/textures/container.jpg") };
	//unsigned int texture2{ loadTexture("D:/Cpp/textures/awesomeface.png",GL_RGBA) };

	//shader.use();
	//shader.setInt("texture1", 0);	// fragShader's texture1 is set with GL_TEXTURE0
	//shader.setInt("texture2", 1);	// texture2 is set with GL_TEXTURE1
	/*******************************************************/

	glEnable(GL_DEPTH_TEST);
	float currentFrame{};
	float lastFrame{};
	float angle{};
	float selfAngle{};
	// every shader has its own configurations, so we need to add another shader to avoid override them

	// define the light and object color and pass them into the shader
	auto lightColor{ glm::vec3{1.0f, 1.0f, 1.0f} }; // white light
	auto objColor{ glm::vec3{1.0f, 0.5f, 0.31f} }; // 
	shader.use();
	//shader.setVec3("lightColor", lightColor); // we don't want the light source been affected
	//shader.setVec3("objectColor", objColor);
	/* Light Struct Controls Material */
	//shader.setVec3("light.ambient", 1.0f, 1.0f, 1.0f); // maintain ambient at a low level
	//shader.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f); // the real color of light source
	shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f); // the color of specular should be full intensity
	shaderView.use();
	shaderView.setVec3("lightColor", lightColor); // we don't want the light source been affected
	shaderView.setVec3("objectColor", objColor);
	shaderGouraud.use();
	shaderGouraud.setVec3("lightColor", lightColor); // we don't want the light source been affected
	shaderGouraud.setVec3("objectColor", objColor);

	for (; !glfwWindowShouldClose(window);)
	{

		currentFrame = glfwGetTime(); // currentFrameTime
		g_deltaTime = currentFrame - lastFrame; // time elapsed in the last frame
		lastFrame = currentFrame; // save current time for the next calculation

		processInput(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, texture1); // bind the data of `texture1` to the GL_TEXTURE0
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, texture2);

		/* Entitiy Controls */
		angle = (float)glfwGetTime() * 25.0f;
		selfAngle = angle * 2.0f;

		// lightColor changes in every frame
		lightColor.x = static_cast<float>(sin(glfwGetTime() * 2.0f));
		lightColor.y = static_cast<float>(sin(glfwGetTime() * 0.7f));
		lightColor.z = static_cast<float>(sin(glfwGetTime() * 1.3f));

		// Control the Color of the entity From the Changes of the light Source
		//auto ambient  { glm::vec3(0.2f) * lightColor };
		//auto diffuse  { glm::vec3(0.5f) * lightColor };
		//auto specular { glm::vec3(1.0f) * lightColor }; // specular lighting should not be influenced by the change with LightColor

		/* if you use the material sheet, ambient's and diffuse's intensity have to set 1.0 */
		/* to get proper material rendering outcome */
		auto ambient{ lightColor };
		auto diffuse{ lightColor };

		glm::mat4 model{ glm::mat4(1.0f) };
		model = glm::translate(model, cubePositions[0]);
		//model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		auto invModel{ glm::inverse(model) }; // for normal matrix

		auto modelLight{ glm::mat4(1.0f) };
		glm::vec3 lightPos{ 2.0f, 1.5f, 2.0f };
		modelLight = glm::scale(modelLight, glm::vec3(0.2f));
		modelLight = glm::rotate(modelLight, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)); // rotate by the Y-axis (Global) because it moved away
		modelLight = glm::translate(modelLight, lightPos);
		glm::vec3 lightPosW{ modelLight * glm::vec4(lightPos,1.0f) }; // don't consider the Roataion, this is the realPos of the light
		/* DON'T multiply this to LightPosW! */
		modelLight = glm::rotate(modelLight, glm::radians(selfAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // rotate by the Y-axis(Local) Rotation

		/* View Controls */
		glm::mat4 view{};
		view = g_myCamera.getViewMatrix();
		auto invViewModel{ glm::inverse(view * model) };

		/* Fov Control and Perspective Projection */
		glm::mat4 perspProj{ glm::perspective(glm::radians(g_myCamera.getFov()), (float)g_width / (float)g_height, 0.1f, 100.0f) };

		/* INJECTIONS */
		// Phong Method (rendering in World Space)
		shader.use();
		shader.setMat4("model", model);
		shader.setMat4("view", view);
		shader.setMat4("projection", perspProj);
		//shader.setVec3("lightColor", lightColor); // define the color of the lightSource
		shader.setMat4("invModel", invModel); // using this to create a Normal Matrix in Shader to fix Normal. Fix non-uniform scaling
		shader.setVec3("light.position", lightPosW); // diffuse lighting
		shader.setVec3("viewerPos", g_myCamera.getPosition()); // for specular lighting
		/* Material Setting */ // JADE
		shader.setVec3("material.ambient",  0.135f, 0.2225f, 0.1575f);
		shader.setVec3("material.diffuse",  0.54f, 0.89f, 0.63f);
		shader.setVec3("material.specular", 0.316228f, 0.316228f, 0.316228f);
		shader.setFloat("material.shiness", 0.1f);
		/* Light Settings */
		shader.setVec3("light.ambient", ambient); // maintain ambient at a low level
		shader.setVec3("light.diffuse", diffuse); // the real color of light source

		// Phong
		// Rendering in View Space, have the same effect with above method
		shaderView.use();
		shaderView.setMat4("model", model);
		shaderView.setMat4("view", view);
		shaderView.setMat4("projection", perspProj);
		shaderView.setMat4("invModel", invViewModel); // this is DIFFERENT with the inversed Matrix above
		shaderView.setVec3("lightPos", lightPosW); // transfom into view Space
		// notice we don't need inject camera's position here, because we're the camera itself

		// Gouraud method, World Space
		shaderGouraud.use();
		shaderGouraud.setMat4("model", model);
		shaderGouraud.setMat4("view", view);
		shaderGouraud.setMat4("projection", perspProj);
		shaderGouraud.setMat4("invModel", invModel); // using this to create a Normal Matrix in Shader to fix Normal
		shaderGouraud.setVec3("lightPos", lightPosW); // diffuse lighting
		shaderGouraud.setVec3("viewerPos", g_myCamera.getPosition());

		// Light Source
		lightShader.use();
		lightShader.setMat4("model", modelLight);
		lightShader.setMat4("view", view);
		lightShader.setMat4("projection", perspProj);
		// change LightSource's Color too
		lightShader.setVec3("lightColor", lightColor);

		/* Rendering */
		lightShader.use(); // activate the Shader
		// Draw the first instance
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		shader.use();
		//shaderView.use();
		//shaderGouraud.use();

		// Draw the first instance
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // Caution: the third parameter is NOT FLOAT!

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	//glDeleteBuffers(1, &EBO);
	glfwTerminate();
	return 0;
}