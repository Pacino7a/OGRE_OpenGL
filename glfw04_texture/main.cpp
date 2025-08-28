#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <learnGL/shader.h>
#include <learnGL/stb_image.h> // IMPLEMENTATION is for this

void framebufferSizeCallBack(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // if the function captured ESC input, we close the window.
		glfwSetWindowShouldClose(window, true);
}

void changeVisibility(GLFWwindow* window, Shader& shader, float& visibility)
{
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS &&  visibility < 1.0f)
	{
		visibility += 0.1f;
		shader.setFloat("visibility", visibility);
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && visibility > 0.1f)
	{
		visibility -= 0.1f;
		shader.setFloat("visibility", visibility);
	}
}

unsigned int loadTexture(const char* path, GLenum format = GL_RGB)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//float borderColor[]{ 1.0f,1.0f,0.0f,1.0f };
	//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // flip the y-axis
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
			format == GL_RGBA ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture: " << path << std::endl;
	}

	stbi_image_free(data);
	return textureID;
}

int main()
{
	glfwInit(); // initialze GLFW (essential!)
	// set openGL as 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window{ glfwCreateWindow(800,600,"LearnOpenGL",NULL,NULL) }; // width, height, name
	if (!window)
	{
		std::cerr << "Sorry, we failed to create a GLFW window.\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window); 

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) // glad manage the pointers of OpenGL
	{
		std::cerr << "Sorry, we failed to initialize GLAD.\n";
		return -1;
	}

	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallBack);

	Shader myShader{ "vertexShader.vs","fragShader.fs" };
	//float vertices[]{
	//	// triangle		   // its color(0~1) // its texture (0~1)
	//	-0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
	//	 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
	//	 0.0f,-0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.0f
	//};

	float vertices[] = {
		// positions          // colors           // texture coords
		 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,	 // top right
		 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,	 // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,	 // bottom left
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f	 // top left 
	};

	unsigned int indices[]{
		0, 1, 2, // first triangle
		2, 3, 0  // second triangle
	};

	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);  // The last argument is not NULL

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0); 
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
	glEnableVertexAttribArray(2);
	//glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(8 * sizeof(float)));
	//glEnableVertexAttribArray(3);
	
	// create & bind current textureObj
	unsigned int textureWood;
	glGenTextures(1, &textureWood);
	glBindTexture(GL_TEXTURE_2D, textureWood);

	// set texture wrapping options (when texture's coordinates  out of the bound [0~1], the fill operation will be activated.)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// if we choose GL_CLAMP_TO_BORDER mode, we need write the following 2 lines
	//float borderColor[]{ 1.0f,1.0f,0.0f,1.0f };
	//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); // you need this statement to fill the empty space

	// set texture Filter
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // we'll use nearest method when the texture downwards
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST); // choose the MipMap nearest and then do the texture linear intreptation for smoothy
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // trillinear filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  // and use Linear Method when the texture upscaling

	// load & generate texture 
	int width, height, nrChannel;
	unsigned char* data{ stbi_load("D:/Cpp/textures/container.jpg",&width,&height,&nrChannel,0) };
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture\n";
	}
	// after we have generated the texture into this program, we release the data
	stbi_image_free(data);
	// You just Created GL_TEXTURE0 Unit

	// GL_TEXTURE1 Unit next, we use LoadFunction here for avoiding the redundancy
	unsigned int textureFace{ loadTexture("D:/Cpp/textures/awesomeface.png",GL_RGBA) };

	// specify the samplers in the fragShader
	myShader.use();
	// status -> shader setting 
	// Only you need to render multiple textures at the same time with the FragShader,
	// you set multiple TextureUnits to the Shader.
	// Otherwise, set one is enough
	myShader.setInt("texture1", 0); // set `texture1` as GL_TEXTURE0
	myShader.setInt("texture2", 1); // set `texture2` as GL_TEXTURE1
	float visibility{ 0 };
	myShader.setFloat("visibility", visibility);

	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		changeVisibility(window, myShader, visibility);
		std::cout << "MixValue: " << visibility << '\n';

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // after the buffer has been cleared, fill this color to the window
		glClear(GL_COLOR_BUFFER_BIT); // clear the color buffer

		// we want to render the unit0£¬ so we need confirm to render
		glActiveTexture(GL_TEXTURE0); // activate texture unit0 port to the shader
		glBindTexture(GL_TEXTURE_2D, textureWood); // and bind the obj to this active texture port(textureWood -> GL_TEXTURE0)
		// and the unit1
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textureFace);

		myShader.use();
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glfwTerminate();

	return 0;
}
