#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include<learnGL/shader.h>
#include<learnGL/stb_image.h>
#include "camera.h"

constexpr int MAJOR_VERSION{3};
constexpr int MINOR_VERSION{3};

constexpr int g_width{ 800 };
constexpr int g_height{ 600 };
auto g_cameraPos{ glm::vec3(0.0f, 0.0f, 3.0f) };
auto g_cameraFront{ glm::vec3(0.0f, 0.0f, -1.0f) }; // real direction of the camera heading (direction vector) (your mouse controls this)
auto g_cameraUp{ glm::vec3(0.0f, 1.0f, 0.0f) };
auto g_deltaTime{ 0.0f };
auto g_pitch{ 0.0f };
auto g_yaw{ -90.0f };
auto g_lastX{ g_width / 2 };
auto g_lastY{ g_height / 2 };
auto g_fov{ 45.0f };
bool g_firstMoveMouse{ true };

Camera g_myCamera{ glm::vec3(0.0f, 0.0f, 3.0f) };


void framebufferSizeCallBack(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	//float cameraSpeed{ 2.5f * g_deltaTime };// normalize the speed
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // if the function captured ESC input, we close the window.
		glfwSetWindowShouldClose(window, true);
	// everytime we press WASD, we update the camera's position
	//if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	//	g_cameraPos += g_cameraFront * cameraSpeed; // sensitivity * real dir-vector (forward+)
	//if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	//	g_cameraPos -= g_cameraFront * cameraSpeed; // setback -
	//if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	//	g_cameraPos -= glm::normalize(glm::cross(g_cameraFront, g_cameraUp)) * cameraSpeed; // using the Right Dir (left  -)
	//if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	//	g_cameraPos += glm::normalize(glm::cross(g_cameraFront, g_cameraUp)) * cameraSpeed; // using the Right Dir (right +)
		
}

void cameraMoveByKeyboard(GLFWwindow* window, float deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		g_myCamera.processKeyboard(Data::forward, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		g_myCamera.processKeyboard(Data::backward, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		g_myCamera.processKeyboard(Data::left, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		g_myCamera.processKeyboard(Data::right, deltaTime);

}

void cursorCallback(GLFWwindow* window, double xpos, double ypos)
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
	
	const float sensitivity{ 0.1f };
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	g_yaw += xOffset;
	g_pitch -= yOffset;

	if (g_pitch > 89.0f) // dir can't be parallel with the World UP
		g_pitch = 89.0f;
	if (g_pitch < -89.0f)
		g_pitch = -89.0f;

	// create a movement vector and excute
	auto direction{ glm::vec3(1.0f) }; // unit vector
	direction.x = cos(glm::radians(g_yaw)) * cos(glm::radians(g_pitch));
	direction.y = sin(glm::radians(g_pitch));
	direction.z = sin(glm::radians(g_yaw)) * cos(glm::radians(g_pitch));
	//std::cout << "Direction:" << direction.x << ", " << direction.y << ", " << direction.z << '\n';
	g_cameraFront = glm::normalize(direction); // mouse take control
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

void scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	g_fov -= yOffset;

	if (g_fov < 1.0f)
		g_fov = 1.0f;
	if (g_fov > 45.0f)
		g_fov = 45.0f;
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
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format == GL_RGBA?GL_RGBA:GL_RGB, GL_UNSIGNED_BYTE, data);
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
	// ---------------- TRANSFORMATION DEMO -----------------------
	//glm::vec4 vec{ 1.0f,0.0f,0.0f,1.0f }; // the original vector £¨includs Homogeneous coordinate)
	//glm::mat4 trans{ glm::mat4(1.0f) }; // create a unit transformation matrix
	//trans = glm::translate(trans, glm::vec3(1.0f, 1.0f, 0.0f)); // build the transformation matrix by given vec3
	//vec = trans * vec; // do translation
	//std::cout << vec.x << ", " << vec.y << ", " << vec.z << '\n';
	// get a transformation matrix includes rotation (based on Z-Axis) and scaling
	glm::mat4 trans2{ glm::mat4(1.0f) };
	trans2 = glm::rotate(trans2, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	trans2 = glm::scale(trans2, glm::vec3(0.5f, 0.5f, 0.5f));
	//vec = trans2 * vec; // this is a mixed-Transformation Matrix (scale first, rotate second)
	//std::cout << vec.x << ", " << vec.y << ", " << vec.z << '\n';


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
	//glfwSetCursorPosCallback(window, cursorCallback); // Cursor Controls
	//glfwSetScrollCallback(window, scrollCallback);
	glfwSetCursorPosCallback(window, cursorCallbackClass); // to register the callback funtions we need a GLOBAL CAMERA
	glfwSetScrollCallback(window, scrollCallbackClass);

	Shader shader{ "vertexShader.vs", "fragShader.fs" };

	//float vertices[]{
	//	// positions          // colors           // texture coords
	//	 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,	 // top right
	//	 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,	 // bottom right
	//	-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,	 // bottom left
	//	-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f	 // top left 
	//};
	//unsigned int indices[]{
	//0, 1, 2, // first triangle
	//2, 3, 0  // second triangle
	//};

	float vertices[] = {
	// x, y, z			  Tex x,y
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
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

	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
	//glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned int texture1{ loadTexture("D:/Cpp/textures/container.jpg") };
	unsigned int texture2{ loadTexture("D:/Cpp/textures/awesomeface.png",GL_RGBA) };

	shader.use();
	shader.setInt("texture1", 0);	// fragShader's texture1 is set with GL_TEXTURE0
	shader.setInt("texture2", 1);	// texture2 is set with GL_TEXTURE1

	// scaling and rotating the Rendering Instances
	//auto transformLocation{ glGetUniformLocation(shader.ID,"transform") };
	//glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(trans2));
	
	glEnable(GL_DEPTH_TEST); // depth test for rendering the pixels correctly with their Z information.

	//float angle_scaling{ 1.0f }; // Wide-angle lens, Telephoto lens
	//float ratio{ 1.0f }; // Stretching instance
	//const float radius{ 10.0f };
	float currentFrame{};
	float lastFrame{};

	for (; !glfwWindowShouldClose(window);)
	{

		currentFrame = glfwGetTime(); // currentFrameTime
		g_deltaTime = currentFrame - lastFrame; // time elapsed in the last frame
		lastFrame = currentFrame; // save current time for the next calculation

		processInput(window);
		cameraMoveByKeyboard(window, g_deltaTime);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1); // bind the data of `texture1` to the GL_TEXTURE0
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);

		// if we want to do the transformations IN THE REAL TIME,
		// we need to write these statements in the rendering loop for updating the transformation matrix
		//glm::mat4 trans3{ glm::mat4(1.0f) }; // initialize this variable in every iteration, otherwise the transformation will go too far
		//trans3 = glm::translate(trans3, glm::vec3(0.5f, -0.5f, 0.0f));
		//trans3 = glm::rotate(trans3, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
		//glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(trans3));
	
		// necessary, get 3 transformation matrices in each frame
		//glm::mat4 model{ glm::mat4(1.0f) };
		//model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f)); // rotate the instance by X-Axis
		glm::mat4 view{};
		//view = glm::translate(view, glm::vec3(0.0f, 0.0f, -7.0f)); // view Matrix is based on Z-Axis from (0,0,0)
		//float camX{ static_cast<float>(sin(glfwGetTime()) * radius) };
		//float camZ{ static_cast<float>(cos(glfwGetTime()) * radius) };
		//view = glm::lookAt(glm::vec3(camX, 0.0f, camZ), glm::vec3(0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0f)); // we create a camera to change the view
		//view = glm::lookAt(g_cameraPos, g_cameraPos + g_cameraFront, g_cameraUp);
		
		
		view = g_myCamera.getViewMatrix();

		// inject the matrices into the shaders
		//auto modelLocation{ glad_glGetUniformLocation(shader.ID,"model") };
		auto viewLocation{ glad_glGetUniformLocation(shader.ID,"view") };
		//auto projectionLocation{ glad_glGetUniformLocation(shader.ID,"projection") };

		// we should put these in the rendering loop for changing in real time
		// injections is needed in every frame
		//glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
		//glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(perspProj));

		// if this matrix is fixed, we can put this outside of the loop
		// (fov, ratio of width and height, near plane's position, far plane's position
		glm::mat4 perspProj{ glm::perspective(glm::radians(g_myCamera.getFov()), ((float)g_width / (float)g_height), 0.1f, 100.0f) };
		shader.setMat4("projection", perspProj);

		shader.use(); // activate the Shader
		// Draw the first instance
		glBindVertexArray(VAO);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // Caution: the third parameter is NOT FLOAT!

		int i{ 0 };
		float angle{};

		// view and projection matrices can be fixed
		// In order to make the instance rotate, translate you can modfied the model matrix only
		for (const auto& position : cubePositions) // render cubes (10)
		{
			glm::mat4 model{ glm::mat4(1.0f) };
			model = glm::translate(model, position);
			angle = 20.0f * i;
			++i;
			if ((i - 1) % 3 == 0) // rotate some cubes in real time
				angle = (float)glfwGetTime() * 25.0f; // make the rotation follow the time elapsed
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f))	;
			shader.setMat4("model", model); // inject
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		
		//glm::mat4 trans4{ glm::mat4(1.0f) };
		//trans4 = glm::translate(trans4, glm::vec3(-0.5f, 0.5f, 0.0f));
		//float sinValue{ static_cast<float>(std::abs(glm::sin(glfwGetTime()))) };
		//trans4 = glm::scale(trans4, glm::vec3(sinValue, sinValue, 1.0f)); // Negative Scaling will make the result reverse
		//glUniformMatrix4fv(transformLocation, 1, GL_FALSE, &trans4[0][0]);

		// Draw the second instance
		//glBindVertexArray(VAO);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glfwTerminate();

	return 0;
}