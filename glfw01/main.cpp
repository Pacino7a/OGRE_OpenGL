#include <glad/glad.h> // GLAD first include, otherwise you will get a compiling error
#include <GLFW/glfw3.h>
#include <iostream>

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
	glfwInit(); // initialze GLFW
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

	// rendering LOOP
	for (;!glfwWindowShouldClose(window);) // check whether the window was indicated to be closed. If so, the loop ends
	{
		processInput(window); // check ESC input in each frame

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // state-setting function (when glClear() was called, the color we set will fill the rendering area
		glClear(GL_COLOR_BUFFER_BIT);		// state-using function (clear the color buffer)

		glfwPollEvents(); // check events
		glfwSwapBuffers(window); // swap the color buffers(2 buffers) for rendering
	}

	glfwTerminate(); // Once the rendering is over, we would like to clean all the GLFW's resources
	return 0;
}