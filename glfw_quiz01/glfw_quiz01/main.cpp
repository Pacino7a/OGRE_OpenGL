#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <array>

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

const char* fragmentShaderSource1 = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{"
"FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n";

const char* fragmentShaderSource2 = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{"
"FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n"
"}\n";

void framebufferSizeCallBack(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // if the function captured ESC input, we close the window.
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
	if (!window)
	{
		std::cerr << "Sorry, we failed to create a GLFW window.\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window); // set the context of `window` as the main context of current thread

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) // glad manage the pointers of OpenGL
	{
		std::cerr << "Sorry, we failed to initialize GLAD.\n";
		return -1;
	}

	glViewport(0, 0, 800, 600); // contains relative position controls (-1~0~1)
	// registering RESIZE. There are many CallBack functions to use. we should register the functions we need BEFORE the rendering loop.
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallBack); // when the size of the window has changed, the area of rendering should follow it too.

	// Create a vertex shader
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	// the first argument is shader we want to compile, the second one is the number of Source code string, the third one is the source code itself
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	//int  success;
	//char infoLog[512];
	//glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	//if (!success)
	//{
	//	glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
	//	std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << '\n';
	//}

	// Create a fragment shader
	unsigned int fragmentShader1; // our fragment shader determines the color of the shape we draw
	fragmentShader1 = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader1, 1, &fragmentShaderSource1, NULL);
	glCompileShader(fragmentShader1);
	// Create the second one
	unsigned int fragmentShader2; // our fragment shader determines the color of the shape we draw
	fragmentShader2 = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader2, 1, &fragmentShaderSource2, NULL);
	glCompileShader(fragmentShader2);

	// Create a Shader Program and link both of the shaders that we created before
	unsigned int shaderProgram1;
	shaderProgram1 = glCreateProgram(); // create a program and return its unique ID
	// notice: attach has a order here
	glAttachShader(shaderProgram1, vertexShader); // attach(copy) the shaders into the program (Vertex Shader goes first)
	glAttachShader(shaderProgram1, fragmentShader1);
	glLinkProgram(shaderProgram1); // link them

	unsigned int shaderProgram2;
	shaderProgram2 = glCreateProgram(); // create a program and return its unique ID
	// notice: attach has a order here
	glAttachShader(shaderProgram2, vertexShader); // attach(copy) the shaders into the program (Vertex Shader goes first)
	glAttachShader(shaderProgram2, fragmentShader2); // NOTICE we changed the fragment shader here
	glLinkProgram(shaderProgram2); // link them

	// because we have linked(copied) the shaders into the program, so we don't need them anymore
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader1);
	glDeleteShader(fragmentShader2);


	//int  success;
	//char infoLog[512];
	//glGetShaderiv(shaderProgram, GL_COMPILE_STATUS, &success);
	//if (!success)
	//{
	//	glGetShaderInfoLog(shaderProgram, 512, NULL, infoLog);
	//	std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" << infoLog << '\n';
	//}


	// Set Vertices (VBO & VAO), send them to the memory of GPU
	//float vertices[]{
	//// the first triangle
	//-0.5f, -0.5f, 0.0f,
	// 0.5f, -0.5f, 0.0f,
	// 0.0f,  0.5f, 0.0f,
	//// the second triangle
	//-0.5f, 0.5f, 0.0f,
	// 0.5f, 0.5f, 0.0f,
	// 0.0f,-0.5f, 0.0f
	//};	
	
	float vertices1[] {
	-0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	 0.0f,  0.5f, 0.0f
	};
	float vertices2[]{
	-0.5f,  0.5f, 0.0f,
	 0.5f,  0.5f, 0.0f,
	 0.0f, -0.5f, 0.0f
	};

	//float vertices[]{
	// 0.5f,  0.5f, 0.0f,  // top right
	// 0.5f, -0.5f, 0.0f,  // bottom right
	//-0.5f, -0.5f, 0.0f,  // bottom left
	//-0.5f,  0.5f, 0.0f   // top left 
	//};

	//unsigned int indices[]{  // note that we start from 0!
	//	0, 1, 3,   // first triangle
	//	1, 2, 3    // second triangle
	//};

	// QUIZ 01
	//unsigned int VAO;
	//unsigned int VBO;

	//glGenVertexArrays(1, &VAO); // we require 1 vertex array with an unique ID
	//glGenBuffers(1, &VBO); // we require 1 buffer with an unique ID

	// bind vertex array object
	//glBindVertexArray(VAO);
	// copy our vertices array in a buffer for OpenGL to use
	//glBindBuffer(GL_ARRAY_BUFFER, VBO); // VBO was binded with ARRAY BUFFER (become its alias)
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// set Vertex attributes
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // the first argument repesent what the vertex is
	//glEnableVertexAttribArray(0); // 0 means this vec3 represents a position, 1 means `color`, etc.
	// unbind EBO and VAO (rarely to do so)
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // you CAN'T do this before VAO is unbinded
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);
	// QUIZ 02
	unsigned int VAOs[2];
	unsigned int VBOs[2];
	////unsigned int EBO;
	glGenVertexArrays(2, VAOs);
	glGenBuffers(2, VBOs);
	//glGenBuffers(1, &EBO); // indicate the order of rendering
	// bind vertex array object
	glBindVertexArray(VAOs[0]);
	// copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]); // VBO was binded with ARRAY BUFFER (become its alias)
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW); // copy the data we have to the buffer, the last argument has 3 types (STREAM, STATIC and DYNAMIC)
	// copy our index array in a element buffer for OpenGL to use
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	// set Vertex attributes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // the first argument repesent what the vertex is
	glEnableVertexAttribArray(0); // 0 means this vec3 represents a position, 1 means `color`, etc.

	glBindVertexArray(VAOs[1]);
	// copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]); // VBO was binded with ARRAY BUFFER (become its alias)
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW); // copy the data we have to the buffer, the last argument has 3 types (STREAM, STATIC and DYNAMIC)
	// set Vertex attributes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // the first argument repesent what the vertex is
	glEnableVertexAttribArray(0); // 0 means this vec3 represents a position, 1 means `color`, etc.
	//unbind EBO and VAO (rarely to do so)
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // you CAN'T do this before VAO is unbinded
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// draw the wireframe instead of a shape with color filled.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// rendering LOOP
	for (; !glfwWindowShouldClose(window);) // check whether the window was indicated to be closed. If so, the loop ends
	{
		processInput(window); // check ESC input in each frame

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // state-setting function (when glClear() was called, the color we set will fill the rendering area)
		glClear(GL_COLOR_BUFFER_BIT);		// state-using function (clear the color buffer)

		glUseProgram(shaderProgram1); // we use different shader program to render different triangles
		//glBindVertexArray(VAO);
		//glDrawArrays(GL_TRIANGLES, 0, 6); // we have 6 vertices to draw this time
		glBindVertexArray(VAOs[0]);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glUseProgram(shaderProgram2); // we control the color of the different shapes by using different shaderProgram (fragment shader is different)
		glBindVertexArray(VAOs[1]);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window); // swap the color buffers(2 buffers) for rendering
		glfwPollEvents(); // check events
	}

	// clean all the resource
	//glDeleteVertexArrays(1, &VAO);
	//glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(2, VAOs);
	glDeleteBuffers(2, VBOs);
	//glDeleteBuffers(1, &EBO);
	glDeleteProgram(shaderProgram1);
	glDeleteProgram(shaderProgram2);

	glfwTerminate(); // Once the rendering is over, we would like to clean all the GLFW's resources
	return 0;
}