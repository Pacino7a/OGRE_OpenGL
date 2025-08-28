#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "shader.h"

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec4 vertexColor;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos, 1.0);\n"
//"	vertexColor = vec4(0.5f, 0.0, 0.0, 1.0);\n" // we set color right here
"	vertexColor = vec4(aColor, 1.0f);\n"
"}\n";

const char* fragmentShaderSource = "#version 330 core\n"
"in vec4 vertexColor;\n" // we recive the color from the shader in the last level
"out vec4 FragColor;\n"
//"uniform vec4 ourColor;\n"
"void main()\n"
"{\n"
"	FragColor = vertexColor;\n" // notice we changed the way of rendering fragment
"}\n";

void framebufferSizeCallBack(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // if the function captured ESC input, we close the window.
		glfwSetWindowShouldClose(window, true);
}

int main()
{
	glfwInit(); // initialze GLFW §Ô§Úessential!)
	// set openGL as 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// we want use the CORE PROFILE
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Mac OSX

	GLFWwindow* window{ glfwCreateWindow(800,600,"LearnOpenGL",NULL,NULL) }; // width, height, name
	if(!window)
	{
		std::cerr << "Sorry, we failed to create a GLFW window.\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window); // set the context of `window` as the main context of current thread

	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) // glad manage the pointers of OpenGL
	{
		std::cerr << "Sorry, we failed to initialize GLAD.\n";
		return -1;
	}

	glViewport(0, 0, 800, 600); // contains relative position controls (-1~0~1)
	// registering RESIZE. There are many CallBack functions to use. we should register the functions we need BEFORE the rendering loop.
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallBack); // when the size of the window has changed, the area of rendering should follow it too.
	
	//unsigned int vertexShader;
	//vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	//glCompileShader(vertexShader);

	//unsigned int fragmentShader;
	//fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	//glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	//glCompileShader(fragmentShader);

	//unsigned int shaderProgram;
	//shaderProgram = glCreateProgram();
	//glAttachShader(shaderProgram, vertexShader);
	//glAttachShader(shaderProgram, fragmentShader);
	//glLinkProgram(shaderProgram);
	//glDeleteShader(vertexShader);
	//glDeleteShader(fragmentShader);

	Shader ourShader{ "D:/Cpp/vistual_studio_save/glfw03_shader/glfw03_shader/3.3_shader.vs",
					  "D:/Cpp/vistual_studio_save/glfw03_shader/glfw03_shader/3.3_shader.fs" };

	float vertices[]{
		// triangle			// its color
		-0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
		 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
		 0.0f,-0.5f, 0.0f, 0.0f, 0.0f, 1.0f
	};

	unsigned int VAO;
	unsigned int VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);  // The last argument is not NULL
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0); // also
	glEnableVertexAttribArray(0); // the argument of current statement have to be the same with the AttributePointer statement's first argument
	// if the color of those vertices are defined in the vertexShader,
	// so we still activate the `position` attribute (layout(location = 0)) to render our shape
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float))); // stride of the both should be 6*sizeof
	glEnableVertexAttribArray(1);											// the offset is the distance from the very beginning to the element we need read


	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // after the buffer has been cleared, fill this color to the window
		glClear(GL_COLOR_BUFFER_BIT); // clear the color buffer


		//float timeValue{ static_cast<float>(glfwGetTime()) };
		//float greenValue{ (sin(timeValue) / 2.0f) + 0.5f };
		//int vertexColorLocation{ glGetUniformLocation(shaderProgram,"ourColor") }; // get uniform's location in the linked shaderProgram
		//glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f); // uniform is a vec4 and can be modified through its location

		//glUseProgram(shaderProgram);

		//ourShader.setFloat("bias", 0.5f);
		ourShader.setBool("flip", true);
		ourShader.use();
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	//glDeleteProgram(shaderProgram);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glfwTerminate();
	return 0;
}