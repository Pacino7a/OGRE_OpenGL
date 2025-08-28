#include <learnGL/glWithMethod.h>
#include <learnGL/camera.h>
#include <learnGL/shader.h>
#include <vector>
#include <map>

// This project includes the usage of Depth, Stencil, Blending, FaceCulling and Framebuffers

constexpr int MAJOR_VERSION{3};
constexpr int MINOR_VERSION{3};
constexpr int NR_POINT_LIGHTS{4};
constexpr int g_width{800};
constexpr int g_height{600};

auto g_lastX{static_cast<double>(g_width) / 2};
auto g_lastY{static_cast<double>(g_height) / 2};
auto g_deltaTime{0.0f};
bool g_firstMoveMouse{true};

int g_framebuffer_width{g_width};
int g_framebuffer_height{g_height};

Camera g_myCamera{glm::vec3(0.0f, 0.0f, 3.0f)};

void updateFramebuffer(unsigned int textureAtt, unsigned int RBO)
{
	glBindTexture(GL_TEXTURE_2D, textureAtt);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_framebuffer_width, g_framebuffer_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	// 更新 depth+stencil renderbuffer 大小
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, g_framebuffer_width, g_framebuffer_height);
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MAJOR_VERSION);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MINOR_VERSION);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window{glfwCreateWindow(g_width, g_height, "LearnOpenGL", NULL, NULL)};
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
	Shader shaderFB("vertexShader_fb.vs", "fragShader_fb.fs");
	// CCW
	float cubeVertices[] = {
		// Back face
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // Bottom-left
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f,	 // top-right
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f,	 // bottom-right
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f,	 // top-right
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // bottom-left
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f,	 // top-left
		// Front face
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-left
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f,	// bottom-right
		0.5f, 0.5f, 0.5f, 1.0f, 1.0f,	// top-right
		0.5f, 0.5f, 0.5f, 1.0f, 1.0f,	// top-right
		-0.5f, 0.5f, 0.5f, 0.0f, 1.0f,	// top-left
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-left
		// Left face
		-0.5f, 0.5f, 0.5f, 1.0f, 0.0f,	 // top-right
		-0.5f, 0.5f, -0.5f, 1.0f, 1.0f,	 // top-left
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,	 // bottom-right
		-0.5f, 0.5f, 0.5f, 1.0f, 0.0f,	 // top-right
										 // Right face
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f,	 // top-left
		0.5f, -0.5f, -0.5f, 0.0f, 1.0f,	 // bottom-right
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f,	 // top-right
		0.5f, -0.5f, -0.5f, 0.0f, 1.0f,	 // bottom-right
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f,	 // top-left
		0.5f, -0.5f, 0.5f, 0.0f, 0.0f,	 // bottom-left
		// Bottom face
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // top-right
		0.5f, -0.5f, -0.5f, 1.0f, 1.0f,	 // top-left
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f,	 // bottom-left
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f,	 // bottom-left
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,	 // bottom-right
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // top-right
		// Top face
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, // top-left
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f,	// bottom-right
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f,	// top-right
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f,	// bottom-right
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, // top-left
		-0.5f, 0.5f, 0.5f, 0.0f, 0.0f	// bottom-left
	};

	float planeVertices[] = {
		// positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
		5.0f, -0.5f, 5.0f, 2.0f, 0.0f,
		-5.0f, -0.5f, 5.0f, 0.0f, 0.0f,
		-5.0f, -0.5f, -5.0f, 0.0f, 2.0f,

		5.0f, -0.5f, 5.0f, 2.0f, 0.0f,
		-5.0f, -0.5f, -5.0f, 0.0f, 2.0f,
		5.0f, -0.5f, -5.0f, 2.0f, 2.0f};

	float cubeVerticesCW[] = {
		// Back face
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
		0.5f, 0.5f, -0.5f, 1.0f, 0.0f,
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
		-0.5f, 0.5f, -0.5f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		// Front face
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
		0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
		0.5f, -0.5f, 0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
		// Left face (y,z)
		-0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
		-0.5f, -0.5f, 0.5f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, 0.5f, -0.5f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
		// Right face
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
		0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
		0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
		// Bottom face (x,z)
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, 0.5f, 1.0f, 1.0f,
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
		0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		// Top face
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
		0.5f, 0.5f, -0.5f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
		-0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
		-0.5f, 0.5f, -0.5f, 0.0f, 0.0f};

	float vegetationVertices[] =
		{
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
			0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
			0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
			-0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f};

	// Cover the Screen
	float screenQuadVertices[] =
		{
			-1.0f, -1.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f, 1.0f,

			1.0f, -1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, 0.0f, 1.0f};

	float rearQuadVertices[] =
		{
			//-0.5f, 0.5f, 0.0f, 0.0f,
			// 0.5f, 0.5f, 1.0f, 0.0f,
			//-0.5f, 1.0f, 0.0f, 1.0f,

			// 0.5f, 0.5f, 1.0f, 0.0f,
			// 0.5f, 1.0f, 1.0f, 1.0f,
			//-0.5f, 1.0f, 0.0f, 1.0f
			-0.3f, 1.0f, 0.0f, 1.0f,
			-0.3f, 0.7f, 0.0f, 0.0f,
			0.3f, 0.7f, 1.0f, 0.0f,

			-0.3f, 1.0f, 0.0f, 1.0f,
			0.3f, 0.7f, 1.0f, 0.0f,
			0.3f, 1.0f, 1.0f, 1.0f};

	std::vector<glm::vec3> vegetation; // Pos
	vegetation.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));
	vegetation.push_back(glm::vec3(1.5f, 0.0f, 0.51f));
	vegetation.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
	vegetation.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
	vegetation.push_back(glm::vec3(0.5f, 0.0f, -0.6f));

	std::vector<glm::vec3> windows; // Pos
	windows.emplace_back(glm::vec3(0.5f, 0.0f, -0.5f));
	windows.emplace_back(glm::vec3(-0.3f, 0.0f, 1.5f));

	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
	glBindVertexArray(0);

	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
	glBindVertexArray(0);

	unsigned int vegetationVAO, vegetationVBO;
	glGenVertexArrays(1, &vegetationVAO);
	glGenBuffers(1, &vegetationVBO);
	glBindVertexArray(vegetationVAO);
	glBindBuffer(GL_ARRAY_BUFFER, vegetationVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vegetationVertices), &vegetationVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
	glBindVertexArray(0);

	unsigned int scnQuadVAO, scnQuadVBO;
	glGenVertexArrays(1, &scnQuadVAO);
	glGenBuffers(1, &scnQuadVBO);
	glBindVertexArray(scnQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, scnQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuadVertices), &screenQuadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
	glBindVertexArray(0);

	unsigned int rearQuadVAO, rearQuadVBO;
	glGenVertexArrays(1, &rearQuadVAO);
	glGenBuffers(1, &rearQuadVBO);
	glBindVertexArray(rearQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, rearQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rearQuadVertices), &rearQuadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
	glBindVertexArray(0);

	unsigned int cubeTex{loadTexture("D:/Cpp/textures/wall.jpg", GL_RGB)};
	unsigned int floorTex{loadTexture("D:/Cpp/textures/Ground_01.png", GL_RGBA)};
	unsigned int grassTex{loadTransparentTex("D:/Cpp/textures/grass.png")};
	unsigned int windowTex{loadTransparentTex("D:/Cpp/textures/blending_transparent_window.png")};

	double currentFrame{};
	double lastFrame{};
	double lastSecond{};
	glm::mat4 model{};
	glm::mat4 view{glm::mat4(1.0f)};
	glm::mat4 projection{glm::mat4(1.0f)};

	/* Depth */ // Rendering Occlusion Relationships
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	/* Stencil */							   // Selective Rendering
	glEnable(GL_STENCIL_TEST);				   // Activate stencil test for rendering
	glStencilFunc(GL_NOTEQUAL, 1, 0xff);	   // Only the instensity is NOT EQUAL to 1 can pass The Stencil Test
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // if stencil and depth tests have passed, update(replace) the stencil value with ref `1`
	// we use glStencilMask(0xff) and glStencilMask(0x00) to control Stencil Write

	/* Culling Faces */ // Cut Faces, imporve performance
	// glEnable(GL_CULL_FACE); // activate
	// glCullFace(GL_BACK); // cull front faces (i.e. ccw faces)
	// glFrontFace(GL_CCW); // front is ccw

	// a framebuffer is a Paintting Board
	unsigned int FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	// Framebuffer Context - Texture
	unsigned int textureAtt;
	glGenTextures(1, &textureAtt);
	glBindTexture(GL_TEXTURE_2D, textureAtt);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_framebuffer_width, g_framebuffer_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); // notice we use NULL here for reserving Mem
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);													  // no mipmap and wrap because we don't need them
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0); // release the bind
	// attach texture to the framebuffer (color buffer?)
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureAtt, 0); // '0'-no mipmaplevel

	// Framebuffer Context - Rendering Buffer (for depth and stencil testing)
	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);																// rendering framebuffer is Write-Only
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, g_framebuffer_width, g_framebuffer_height); // GL_DEPTH24_STENCIL8 as the internal format
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO); // attach it to the Framebuffer

	// BTW
	// we can make Depth(24bits) and Stencil(8bit) a single Texture
	// glTexImage2D(
	//	GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0,
	//	GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL
	//);
	// To attach a depth and stencil buffer as one texture we use the GL_DEPTH_STENCIL_ATTACHMENT type
	// and configure the texture's formats to contain combined depth and stencil values.
	// glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, textureDS, 0);

	unsigned int FBOrear;
	glGenFramebuffers(1, &FBOrear);
	glBindFramebuffer(GL_FRAMEBUFFER, FBOrear);

	// Framebuffer Context - Texture
	unsigned int textureRearView;
	glGenTextures(1, &textureRearView);
	glBindTexture(GL_TEXTURE_2D, textureRearView);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_framebuffer_width, g_framebuffer_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); // notice we use NULL here for reserving Mem
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);													  // no mipmap and wrap because we don't need them
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0); // release the bind
	// attach texture to the framebuffer (color buffer?)
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureRearView, 0); // '0'-no mipmaplevel

	// Framebuffer Context - Rendering Buffer (for depth and stencil testing)
	unsigned int RBOrear;
	glGenRenderbuffers(1, &RBOrear);
	glBindRenderbuffer(GL_RENDERBUFFER, RBOrear);															// rendering framebuffer is Write-Only
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, g_framebuffer_width, g_framebuffer_height); // GL_DEPTH24_STENCIL8 as the internal format
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBOrear); // attach it to the Framebuffer

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "ERROR::FRAMEBUFFER:: framebuffer is not complete!\n";
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // activate default framebuffer for main rendering, and make sure we are
										  // not accidently rendering to the wrong framebuffer
	// glDeleteBuffers(1, &FBO); // when you have done with the buffer

	// for rendering the Rendered Scene Texture
	shaderFB.use();
	shaderFB.setInt("screenTexture1", 0);

	shader.use();
	shader.setInt("texture1", 0);
	shader.setBool("drawFrame", false);

	for (; !glfwWindowShouldClose(window);)
	{
		currentFrame = glfwGetTime();								// currentFrameTime
		g_deltaTime = static_cast<float>(currentFrame - lastFrame); // time elapsed in the last frame
		lastFrame = currentFrame;									// save current time for the next calculation

		if (currentFrame - lastSecond > 1)
		{
			lastSecond = currentFrame;
			auto fps{1 / g_deltaTime};
			std::ostringstream strStream;
			strStream << "LearnOpenGL FPS:" << fps << "";
			glfwSetWindowTitle(window, strStream.str().c_str());
		}

		processInput(window);
		// when you change your window's size, not only you need to update your glviewport,
		// but also update your framebuffer's size
		// especially when you use their attached textures to render your scene
		updateFramebuffer(textureAtt, RBO);
		updateFramebuffer(textureRearView, RBOrear);

		// First Pass (Render the Scene to the texture attched to our Framebuffer)
		glBindFramebuffer(GL_FRAMEBUFFER, FBO); // Scene -> Framebuffer's texture
		glEnable(GL_DEPTH_TEST);				// if you don't do this, Occlusion relationships can become confusing
												// but when you want show something on the front, you can disable this
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		shader.use(); // you need write this, because there is a swap of the shaders during the rendering loop (otherwise it will cause rendering failure)
		view = g_myCamera.getViewMatrix();
		projection = glm::perspective(g_myCamera.getFov(), (float)g_width / g_height, 0.1f, 100.0f);
		shader.setFloat("near", 0.1f);
		shader.setFloat("far", 100.0f);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		// floor
		// glDepthMask(GL_FALSE);
		glStencilMask(0x00); // render floors as usual, we only care about the containers. We set its mask to 0x00 to not write to the stencil buffer.
		glBindVertexArray(planeVAO);
		glBindTexture(GL_TEXTURE_2D, floorTex);
		shader.setMat4("model", glm::mat4(1.0f));
		glDrawArrays(GL_TRIANGLES, 0, 6); // floor just has a Surface
		// glDepthMask(GL_TRUE);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
		model = glm::scale(model, glm::vec3(1.5f));
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// cube
		/* Depth Test */
		// glDepthMask(GL_FALSE); // Deactivate the Depth Test, can easily be covered by other Entities front of it
		/* Cull Faces */
		glEnable(GL_CULL_FACE); // activate
		glCullFace(GL_BACK);	// cull front faces (i.e. ccw faces)
		glFrontFace(GL_CCW);	// front is ccw

		glBindVertexArray(cubeVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTex);
		/* Stencil Test */
		// The first Render Pass, Just Render as Usual
		glStencilFunc(GL_ALWAYS, 1, 0xFF); // should we render this fragment? ---> (pixelIntensity * 0xFF) EQUAL? (1 * 0xFF),
										   // if so, replace the stencil value with ref value `1`
		glStencilMask(0xFF);			   // activate all stencil bits for writing (following the Op's Direction, REPLACE the stencil value of Rendered Area)
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-1.0f, 0.005f, -1.0f)); // set Y a little higher to avoid Z-fighting
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36); // the cube area's stencil values are all `1`
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(2.0f, 0.005f, 0.0f));
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// glDepthMask(GL_TRUE);

		// The second Render Pass, Render the Borders (Expanded Cube, Stencil Value Not equal `1` for Rendering)
		shader.setBool("drawFrame", true);
		glStencilFunc(GL_NOTEQUAL, 1, 0xff); // the cube's stencil value is 1, but only the area which stencil is NOT EQUAL `1` can pass and render
		glStencilMask(0x00);				 // ban write, avoid to break cube's mask
		glDisable(GL_DEPTH_TEST);			 // make sure the border(Even the entire outerShape) is always Visiable

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-1.0f, 0.005f, -1.0f));
		model = glm::scale(model, glm::vec3(1.1f));
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36); // because of the MASK of `1`, we can only render the border part
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(2.0f, 0.005f, 0.0f));
		model = glm::scale(model, glm::vec3(1.1f));
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// restore the Stencil, Depth and CullFace
		shader.setBool("drawFrame", false);
		glStencilMask(0xff);
		glStencilFunc(GL_ALWAYS, 1, 0xff);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glBindVertexArray(0);

		glBindVertexArray(vegetationVAO);
		glBindTexture(GL_TEXTURE_2D, grassTex);
		shader.setBool("drawGrass", true);
		for (const auto &vegePos : vegetation)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, vegePos);
			// model = glm::scale(model, glm::vec3(0.5f));
			shader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		shader.setBool("drawGrass", false);

		glEnable(GL_BLEND);
		// glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // depth test will make this not so `Transparent`, you must render the entities from far to near.
		// glBlendFuncSeparate(); // Set RGB and Alpha separately
		// glBlendEquation(GL_FUNC_ADD); // usually FUNC_ADD
		glBindTexture(GL_TEXTURE_2D, windowTex);
		std::map<float, glm::vec3> sorted_windows{};
		float distance{};
		for (const auto &winPos : windows) // get a order(near->far) of the windows to be rendered
		{
			distance = glm::length(g_myCamera.getPosition() - winPos);
			sorted_windows[distance] = winPos;
		}
		for (auto it{sorted_windows.rbegin()}; it != sorted_windows.rend(); ++it) // Rendering follows the distance to the Camera (Far to Near)
		{																		  // reverse it (far -> near) for rendering
			model = glm::mat4(1.0f);
			model = glm::translate(model, it->second);
			shader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			// glEnable(GL_DEPTH_TEST);
		}
		glDisable(GL_BLEND);

		// Rear-View
		glBindFramebuffer(GL_FRAMEBUFFER, FBOrear); // Scene -> Framebuffer's texture
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		view = g_myCamera.getReverseViewMatrix();
		shader.setMat4("view", view); // change the view
		glStencilMask(0x00);		  // render floors as usual, we only care about the containers. We set its mask to 0x00 to not write to the stencil buffer.
		glBindVertexArray(planeVAO);
		glBindTexture(GL_TEXTURE_2D, floorTex);
		shader.setMat4("model", glm::mat4(1.0f));
		glDrawArrays(GL_TRIANGLES, 0, 6); // floor just has a Surface
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
		model = glm::scale(model, glm::vec3(1.5f));
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// cube
		/* Depth Test */
		// glDepthMask(GL_FALSE); // Deactivate the Depth Test, can easily be covered by other Entities front of it
		/* Cull Faces */
		glEnable(GL_CULL_FACE); // activate
		glCullFace(GL_BACK);	// cull front faces (i.e. ccw faces)
		glFrontFace(GL_CCW);	// front is ccw

		glBindVertexArray(cubeVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTex);
		/* Stencil Test */
		// The first Render Pass, Just Render as Usual
		glStencilFunc(GL_ALWAYS, 1, 0xFF); // should we render this fragment? ---> (pixelIntensity * 0xFF) EQUAL? (1 * 0xFF)
		glStencilMask(0xFF);			   // activate all stencil bits for writing (Enable only on usual rendering of Container)
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-1.0f, 0.005f, -1.0f)); // set Y a little higher to avoid Z-fighting
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(2.0f, 0.005f, 0.0f));
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// glDepthMask(GL_TRUE);

		// The second Render Pass, Render the Borders (Expanded Cube, Stencil Value Not equal `1` for Rendering)
		shader.setBool("drawFrame", true);
		glStencilFunc(GL_NOTEQUAL, 1, 0xff);
		glStencilMask(0x00);	  // ban write
		glDisable(GL_DEPTH_TEST); // make sure the border(Even the entire outerShape) is always Visiable

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-1.0f, 0.005f, -1.0f));
		model = glm::scale(model, glm::vec3(1.1f));
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(2.0f, 0.005f, 0.0f));
		model = glm::scale(model, glm::vec3(1.1f));
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// restore the Stencil, Depth and CullFace
		shader.setBool("drawFrame", false);
		glStencilMask(0xff);
		glStencilFunc(GL_ALWAYS, 1, 0xff);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glBindVertexArray(0);

		glBindVertexArray(vegetationVAO);
		glBindTexture(GL_TEXTURE_2D, grassTex);
		shader.setBool("drawGrass", true);
		for (const auto &vegePos : vegetation)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, vegePos);
			// model = glm::scale(model, glm::vec3(0.5f));
			shader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		shader.setBool("drawGrass", false);

		glEnable(GL_BLEND);
		// glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // depth test will make this not so `Transparent`, you must render the entities from far to near.
		// glBlendFuncSeparate(); // Set RGB and Alpha separately
		// glBlendEquation(GL_FUNC_ADD); // usually FUNC_ADD
		glBindTexture(GL_TEXTURE_2D, windowTex);
		sorted_windows = {};
		distance = 0;
		for (const auto &winPos : windows) // get a order(near->far) of the windows to be rendered
		{
			distance = glm::length(g_myCamera.getPosition() - winPos);
			sorted_windows[distance] = winPos;
		}
		for (auto it{sorted_windows.rbegin()}; it != sorted_windows.rend(); ++it) // Rendering follows the distance to the Camera (Far to Near)
		{																		  // reverse it (far -> near) for rendering
			model = glm::mat4(1.0f);
			model = glm::translate(model, it->second);
			shader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			// glEnable(GL_DEPTH_TEST);
		}
		glDisable(GL_BLEND);

		// Second Pass (Output the Rendered Scene Texture which is writed through our designed framebuffer to the monitor)
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
		glDisable(GL_DEPTH_TEST);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Now the entire scene is rendered to a single texture
		shaderFB.use(); // we can use this shader to do some Post-Effects
		glBindVertexArray(scnQuadVAO);
		glBindTexture(GL_TEXTURE_2D, textureAtt); // render Scene to the Monitor
		// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(rearQuadVAO);
		glBindTexture(GL_TEXTURE_2D, textureRearView); // and the rear-view
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteVertexArrays(1, &scnQuadVAO);
	glDeleteVertexArrays(1, &rearQuadVAO);

	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &planeVBO);
	glDeleteBuffers(1, &scnQuadVBO);
	glDeleteBuffers(1, &rearQuadVBO);

	glDeleteRenderbuffers(1, &RBO);
	glDeleteRenderbuffers(1, &RBOrear);

	glDeleteFramebuffers(1, &FBO);
	glDeleteFramebuffers(1, &FBOrear);

	glfwTerminate();
	return 0;
}
