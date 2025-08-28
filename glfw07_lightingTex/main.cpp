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
//bool g_isXray{ false };

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

	//if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
	//	g_isXray = !g_isXray;

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
		// back  (fixed -z)
		//-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		// 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		// 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		// 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		//-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		//-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		// front (fixed +z)
		//-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		// 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		// 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		// 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		//-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		//-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		// left  (fixed -x)
		//-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		//-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		//-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		//-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		//-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		//-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		// right  (fixed +x)
		// 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		// 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		// 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		// 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		// 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		// 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		// bottom  (fixed -y)
		//-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		// 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		// 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		// 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		//-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		//-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		// head  (fixed +y)
		//-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		// 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		// 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		// 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		//-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		//-0.5f,  0.5f, -0.5f,  0.0f, 1.0f

	// coordinates,			// normal		   Tex x, y
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,

	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
	
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
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
	//Shader shader{ "vertexShader.vs", "fragShader.fs" };
	//Shader shader{ "vertexShader.vs", "fragShaderDirectional.fs" };
	Shader shader{ "vertexShader.vs", "fragShaderSpotLight.fs" };

	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // read normal coords
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); // read texture coords
	glEnableVertexAttribArray(2); 

	unsigned int texture1{ loadTexture("D:/Cpp/textures/container2.png", GL_RGBA) };
	unsigned int texture2{ loadTexture("D:/Cpp/textures/container2_specular.png", GL_RGBA) };
	unsigned int texture3{ loadTexture("D:/Cpp/textures/matrix.jpg", GL_RGB) };

	/* uniform sampler2D texturex */
	//shader.use();
	//shader.setInt("texture1", 0);	// fragShader's texture1 is set with GL_TEXTURE0
	//shader.setInt("texture2", 1);	// texture2 is set with GL_TEXTURE1
	/*******************************************************/

	glEnable(GL_DEPTH_TEST);
	float currentFrame{};
	float lastFrame{};
	float angle{};
	float selfAngle{};
	float lastSecond{};

	// every shader has its own configurations, so we need to add another shader to avoid override them

	// define the light and object color and pass them into the shader
	auto lightColor{ glm::vec3{1.0f, 1.0f, 1.0f} }; // white light
	shader.use();
	/* Light Struct Controls Material (FIXED LightColor) */
	//shader.setVec3("light.ambient", lightColor); // maintain ambient at a low level
	//shader.setVec3("light.diffuse", lightColor); // the real color of light source
	
	/* Material Setting */ // JADE
	//shader.setVec3("material.ambient", 0.135f, 0.2225f, 0.1575f);
	//shader.setVec3("material.diffuse", 0.54f, 0.89f, 0.63f);
	//shader.setVec3("material.specular", 0.316228f, 0.316228f, 0.316228f);
	shader.setFloat("material.shiness", 0.25f); // 32
	// Using Texture to replace them
	shader.setInt("material.diffuse", 0);
	shader.setInt("material.specular", 1);
	shader.setInt("material.emission", 2);

	for (; !glfwWindowShouldClose(window);)
	{

		currentFrame = glfwGetTime(); // currentFrameTime
		g_deltaTime = currentFrame - lastFrame; // time elapsed in the last frame
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

		/* X-Ray */
		//if(g_isXray)
		//{
		//	shader.setBool("isXrayMode", true);
		//	shader.setVec2("screenSize", g_width, g_height);
		//	shader.setFloat("xRayRadius", 100.0f);
		//}
		//else
		//	shader.setBool("isXrayMode", false);

		/* Create 3 Transformation Matices */
		angle = (float)glfwGetTime() * 25.0f;
		selfAngle = angle * 2.0f;

		glm::mat4 model{ glm::mat4(1.0f) };
		model = glm::translate(model, cubePositions[0]);
		//model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		auto invModel{ glm::inverse(model) }; // for normal matrix

		auto modelLight{ glm::mat4(1.0f) };
		glm::vec3 lightPos{ 2.0f, 1.5f, -10.0f };
		//modelLight = glm::rotate(modelLight, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)); // rotate by the Y-axis (Global) because it moved away
		modelLight = glm::translate(modelLight, lightPos);
		modelLight = glm::scale(modelLight, glm::vec3(0.2f));
		glm::vec3 lightPosW{ modelLight * glm::vec4(1.0f) }; // don't consider the Roataion, this is the realPos of the light
		/* DON'T multiply this to LightPosW! */
		modelLight = glm::rotate(modelLight, glm::radians(selfAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // rotate by the Y-axis(Local) Rotation

		/* View */
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
		shader.setMat4("invModel", invModel); // using this to create a Normal Matrix in Shader to fix Normal. Fix non-uniform scaling
		//shader.setVec3("viewerPos", g_myCamera.getPosition()); // for specular lighting

		/* Using Texture to replace Material Setting */
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1); // replace diffuse and ambient
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2); // replace specular
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, texture3); // add emission

		// abs(sin(glfwGetTime()))
		shader.setVec3("emissionControl", glm::vec3(abs(sin(glfwGetTime())))); // we use abs to prevent color reverse
		shader.setFloat("time", glfwGetTime()); // pass in time for making emission texture run

		// Changing LightColor
		//lightColor.x = static_cast<float>(sin(glfwGetTime() * 2.0f));
		//lightColor.y = static_cast<float>(sin(glfwGetTime() * 0.7f));
		//lightColor.z = static_cast<float>(sin(glfwGetTime() * 1.3f));
		
		// Control the Color of the entity From the Changes of the light Source
		auto ambient{ glm::vec3(0.2f) * lightColor };
		auto diffuse{ glm::vec3(0.5f) * lightColor };

		/* Lighting the Entity (Point) */
		//shader.setVec3("light.position", lightPosW); // diffuse lighting
		//shader.setVec3("light.ambient", ambient); // maintain ambient at a low level
		//shader.setVec3("light.diffuse", diffuse); // the real color of light source
		//shader.setVec3("light.specular", glm::vec3(0.5f));
		/* Set Light Attenuation */
		//shader.setFloat("light.constant", 1.0f);
		//shader.setFloat("light.linear", 0.09f);
		//shader.setFloat("light.quadratic", 0.032f);
		/* Use DirectionalLight here */
		//shader.setVec3("Dlight.direction", -0.2f, -1.0f, -0.3f);
		//shader.setVec3("Dlight.ambient", ambient); // maintain ambient at a low level
		//shader.setVec3("Dlight.diffuse", diffuse); // the real color of light source
		//shader.setVec3("Dlight.specular", glm::vec3(0.5f));
		
		/* Injections of Directional Light and SpotLight */
		shader.setVec3("dl.direction", -0.2f, -1.0f, -0.3f);
		shader.setVec3("dl.ambient", ambient);
		shader.setVec3("dl.diffuse", diffuse);
		shader.setVec3("dl.specular", glm::vec3(0.5f));
		shader.setVec3("flash.position", g_myCamera.getPosition());
		shader.setVec3("flash.direction", g_myCamera.getFront());
		shader.setVec3("flash.ambient", ambient);
		shader.setVec3("flash.diffuse", diffuse);
		shader.setVec3("flash.specular", glm::vec3(0.5f));
		shader.setFloat("flash.cutOff", glm::cos(glm::radians(12.5f))); // constrain by cos(Phi)
		shader.setFloat("flash.outerCutOff", glm::cos(glm::radians(17.5f))); // constrain by cos(Gamma)
		shader.setFloat("flash.constant", 1.0f);
		shader.setFloat("flash.linear", 0.09f);
		shader.setFloat("flash.quadratic", 0.032f);

		/* Injections of Point Light*/
		//lightShader.use();
		//lightShader.setMat4("model", modelLight);
		//lightShader.setMat4("view", view);
		//lightShader.setMat4("projection", perspProj);
		// change LightSource's Color too
		//lightShader.setVec3("lightColor", lightColor);

		/* Rendering Point LightSource */
		//lightShader.use(); // activate the Shader
		//glBindVertexArray(VAO);
		//glDrawArrays(GL_TRIANGLES, 0, 36);
		//shader.use();
		//glBindVertexArray(VAO);
		//glDrawArrays(GL_TRIANGLES, 0, 36);

		/* Rendering Entities */
		int i{ 0 };
		for (const auto& position : cubePositions)
		{
			shader.use();
			auto localModel{ glm::mat4(1.0f) };
			localModel = glm::translate(localModel, position);
			if (i % 3 == 0)
				localModel = glm::rotate(localModel, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			auto localInvModel{ glm::inverse(localModel) };
			shader.setMat4("model", localModel);
			shader.setMat4("invModel", localInvModel);
			shader.setVec3("viewerPos", g_myCamera.getPosition());
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			++i;
		}


		/* XRay Test */
		//if(!g_isXray)
		//{
		//	shader.use();
		//	glBindVertexArray(VAO);
		//	glDrawArrays(GL_TRIANGLES, 0, 36);
		//}
		//else
		//{
		//	// first pass
		//	shader.use();

		//	shader.setBool("isSecondPass", false);
		//	glCullFace(GL_BACK);
		//	glDepthMask(GL_TRUE);
			// rendering entity
		//	glBindVertexArray(VAO);
		//	glDrawArrays(GL_TRIANGLES, 0, 36);

		//	// X-ray second pass
		//	shader.setBool("isSecondPass", true);
		//	glCullFace(GL_FRONT);
		//	glDepthMask(GL_FALSE);
		//	glBindVertexArray(VAO);
		//	glDrawArrays(GL_TRIANGLES, 0, 36);
		//	
		//	glDepthMask(GL_TRUE);
		//	glCullFace(GL_BACK);
		//}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	//glDeleteBuffers(1, &EBO);
	glfwTerminate();
	return 0;
}