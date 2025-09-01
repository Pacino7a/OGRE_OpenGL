#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <random>
#include <filesystem>
#include <learnGL/camera.h>
#include <learnGL/stb_image.h>
#include <learnGL/config.h>
#include <learnGL/glWithMethod.h>

float lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

void framebufferSizeCallBack(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);

	g_framebuffer_width = width;
	g_framebuffer_height = height;
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

unsigned int loadTexture(const char* path, bool reverse, GLenum format)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) 
	{
		std::cerr << "OpenGL Error after glGenTextures: " << error << std::endl;
	}
	glBindTexture(GL_TEXTURE_2D, textureID);
	error = glGetError();
	if (error != GL_NO_ERROR) 
	{
		std::cerr << "OpenGL Error after glBindTexture: " << error << std::endl;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//float borderColor[]{ 0.0f,0.0f,0.5f,1.0f };
	//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nClrChannel;
	stbi_set_flip_vertically_on_load(reverse);
	unsigned char* data{ stbi_load(path, &width, &height, &nClrChannel, 0) };
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, ((format == GL_RGBA) || (format == GL_SRGB_ALPHA)) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "We FAILED to load Texture!\n";
	}
	stbi_image_free(data);
	return textureID;
}

unsigned int loadHDRadiance(const char* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		std::cerr << "OpenGL Error after glGenTextures: " << error << std::endl;
	}
	glBindTexture(GL_TEXTURE_2D, textureID);
	error = glGetError();
	if (error != GL_NO_ERROR)
	{
		std::cerr << "OpenGL Error after glBindTexture: " << error << std::endl;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nClrChannel;
	stbi_set_flip_vertically_on_load(true);
	float* data{ stbi_loadf(path, &width, &height, &nClrChannel, 0) }; // CAUTION: loadf here and data is float*
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
	}
	else
	{
		std::cout << "We FAILED to load HDR image!\n";
	}
	stbi_image_free(data);
	return textureID;
}

unsigned int loadNormalTex(const char* path, bool reverse, GLenum format)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		std::cerr << "OpenGL Error after glGenTextures: " << error << std::endl;
	}
	glBindTexture(GL_TEXTURE_2D, textureID);
	error = glGetError();
	if (error != GL_NO_ERROR)
	{
		std::cerr << "OpenGL Error after glBindTexture: " << error << std::endl;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nClrChannel;
	stbi_set_flip_vertically_on_load(reverse);
	unsigned char* data{ stbi_load(path, &width, &height, &nClrChannel, 0) };
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, (format == GL_RGBA) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
		// Notice we have BANED Mipmap here!
	}
	else
	{
		std::cout << "We FAILED to load Texture!\n";
	}
	stbi_image_free(data);
	return textureID;
}

unsigned int loadParallaxTex(const char* path, bool reverse, GLenum format)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		std::cerr << "OpenGL Error after glGenTextures: " << error << std::endl;
	}
	glBindTexture(GL_TEXTURE_2D, textureID);
	error = glGetError();
	if (error != GL_NO_ERROR)
	{
		std::cerr << "OpenGL Error after glBindTexture: " << error << std::endl;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height, nClrChannel;
	stbi_set_flip_vertically_on_load(reverse);
	unsigned char* data{ stbi_load(path, &width, &height, &nClrChannel, 1) };
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
		// Notice we have BANED Mipmap here!
	}
	else
	{
		std::cout << "We FAILED to load Texture!\n";
	}
	stbi_image_free(data);
	return textureID;
}

unsigned int loadTransparentTex(const char* path, bool reverse)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nClrChannel;
	stbi_set_flip_vertically_on_load(reverse);
	unsigned char* data{ stbi_load(path, &width, &height, &nClrChannel, 0) };
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "We FAILED to load Texture!\n";
	}
	stbi_image_free(data);
	return textureID;
}

unsigned int textureFromFile(const char* path, const std::string& directory, bool gamma, bool reverse)
{
	std::string fileName{ path };
	fileName = directory + '/' + fileName;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nClrChannel;
	stbi_set_flip_vertically_on_load(reverse);
	unsigned char* data{ stbi_load(fileName.c_str(), &width, &height, &nClrChannel, 0) };
	if (data)
	{
		GLenum format{ GL_SRGB };
		if (nClrChannel == 1)
			format = GL_RED;
		else if (nClrChannel == 3)
			format = GL_SRGB;
		else if (nClrChannel == 4)
			format = GL_SRGB_ALPHA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		switch (format)
		{
		case GL_SRGB:
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			break;
		case GL_SRGB_ALPHA:
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			break;
		case GL_RED:
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
			break;
		default:
			std::cerr << "Fatal Error: LoadTexture has been Modified!\n";
			return -1;
		}
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//float borderColor[]{ 0.0f,0.0f,0.5f,1.0f };
		//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		std::cout << "We didn't read any Texture of Current Model!\n";
	}
	stbi_image_free(data);
	return textureID;
}

std::string getFullPath(const std::string& relativePath)
{
	return std::filesystem::current_path().string() + "/" + relativePath;
}

unsigned int loadCubeMapTextures(const std::vector<std::string>& texture_faces)
{
	// Because a cubemap contains 6 textures, one for each face,
	// we have to call glTexImage2D six times with their parameters set
	unsigned int cubeTextureMapID;
	glGenTextures(1, &cubeTextureMapID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTextureMapID);

	int width, height, nrChannels;
	//stbi_set_flip_vertically_on_load(true);
	unsigned char* data;
	for (std::size_t i{ 0 }; i < texture_faces.size(); ++i)
	{
		data = stbi_load(texture_faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << texture_faces[i] << std::endl;
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return cubeTextureMapID;

}


#ifdef __MULTISAMPLE__
void updateFramebuffer(const unsigned int textureAtt, const unsigned int RBO, const bool isMultisample)
{
	if(!isMultisample)
	{
		glBindTexture(GL_TEXTURE_2D, textureAtt);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_framebuffer_width, g_framebuffer_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		// 更新 depth+stencil renderbuffer 大小
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, g_framebuffer_width, g_framebuffer_height);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureAtt);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MULTISAMPLER, GL_RGB, g_framebuffer_width, g_framebuffer_height, GL_TRUE);

		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, MULTISAMPLER, GL_DEPTH24_STENCIL8, g_framebuffer_width, g_framebuffer_height);
	}
}
#endif // __MULTISAMPLE__

#ifndef __MULTISAMPLE__
void updateFramebuffer(const unsigned int textureAtt, const unsigned int RBO)
{
	glBindTexture(GL_TEXTURE_2D, textureAtt);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_framebuffer_width, g_framebuffer_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	// 更新 depth+stencil renderbuffer 大小
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, g_framebuffer_width, g_framebuffer_height);
}
#endif //__MULTISAMPLE__

std::vector<glm::mat4> getShadowCubeMatrices(const glm::vec3& lightPos, float fov, float aspect, float near_plane, float far_plane)
{
	// Point light Matrices (6 dirs)
	glm::mat4 lightProjection = glm::perspective(glm::radians(fov), aspect, near_plane, far_plane); // to create a point shadow, we need a perspective LightSpace
	std::vector<glm::mat4> shadowTransforms;
	shadowTransforms.reserve(6);
	// Positive X
	shadowTransforms.emplace_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
	// Negative X
	shadowTransforms.emplace_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
	// Positive Y
	shadowTransforms.emplace_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	// Negative Y
	shadowTransforms.emplace_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	// Positive Z
	shadowTransforms.emplace_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
	// Negative Z
	shadowTransforms.emplace_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)));

	return shadowTransforms;
}

glm::vec3 calculateTangents(const glm::vec3& edge1, const glm::vec3& edge2,
							const glm::vec2& deltaUV1, const glm::vec2& deltaUV2)
{
	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	glm::vec3 tangent1{};
	glm::vec3 bitangent1{};
	tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x); // deltaV2 and deltaV1
	tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

	// No necessary to do this
	//bitangent1.x = f * (deltaUV1.x * edge2.x - deltaUV2.x * edge1.x); // deltaU1 and deltaU2
	//bitangent1.y = f * (deltaUV1.x * edge2.y - deltaUV2.x * edge1.y);
	//bitangent1.z = f * (deltaUV1.x * edge2.z - deltaUV2.x * edge1.z);

	return tangent1;
}

// CAUTION!!
// We only need to use SRGB format when we LOAD TEXTURES!!!
std::vector<unsigned int> getFBO_Tex(int SCR_WIDTH, int SCR_HEIGHT)
{
	unsigned int FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	// Framebuffer Context - Texture
	unsigned int textureAtt;
	glGenTextures(1, &textureAtt);
	glBindTexture(GL_TEXTURE_2D, textureAtt);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); // notice we use NULL here for reserving Mem
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);													  // no mipmap and wrap because we don't need them
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0); // release the bind
	// attach texture to the framebuffer (color buffer?)
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureAtt, 0); // '0'-no mipmaplevel

	// Framebuffer Context - Rendering Buffer (for depth and stencil testing)
	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);																// rendering framebuffer is Write-Only
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // GL_DEPTH24_STENCIL8 as the internal format
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

	return { FBO, textureAtt, RBO };
}

std::vector<unsigned int> getMutisampleFBO_Tex(int sampler, int SCR_WIDTH, int SCR_HEIGHT)
{
	/* MULTISAMPLED FRAMEBUFFER */
	unsigned int multisampledFBO;
	glGenFramebuffers(1, &multisampledFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, multisampledFBO);
	unsigned int tex; // multisample texture attachment
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, sampler, GL_RGB, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex, 0);
	unsigned int RBO; // and multisample renderBuffer
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, sampler, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return { multisampledFBO, tex, RBO };
}

std::pair<unsigned int, unsigned int> getDMsFBO_MAP(int SHADOW_WIDTH, int SHADOW_HEIGHT)
{
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap); // a Point Light, need to use a CUBEMAP!
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	/* A framebuffer object however is not complete without a color buffer
	  so we need to explicitly tell OpenGL we're not going to render any color data. */
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Framebuffer not complete!\n";
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return { depthMapFBO, depthMap };
}

// depth cube map
std::pair<unsigned int, unsigned int> getDCMsFBO_MAP(int SHADOW_WIDTH, int SHADOW_HEIGHT)
{
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthMap); // a Point Light, need to use a CUBEMAP!
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
	/* A framebuffer object however is not complete without a color buffer
	  so we need to explicitly tell OpenGL we're not going to render any color data. */
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Framebuffer not complete!\n";
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return { depthMapFBO, depthMap };
}

unsigned int getUBO(std::size_t structureSize)
{
	unsigned int UBO;
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, structureSize, NULL, GL_STATIC_DRAW);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return UBO;
}

std::vector<unsigned int> getHDR_FBO_Tex(int SCR_WIDTH, int SCR_HEIGHT)
{
	unsigned int hdrFBO;
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	unsigned int hdrTex;
	glGenTextures(1, &hdrTex);
	glBindTexture(GL_TEXTURE_2D, hdrTex); // a Point Light, need to use a CUBEMAP!
	glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0 , GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrTex, 0);

	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Framebuffer not complete!\n";
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return { hdrFBO, hdrTex, RBO};
}

// Bloom Post-Effect only need a vector size 2.
std::vector<unsigned int> getHDR_FBO_MutipleTex(std::vector<unsigned int>& hdrTextures, int SCR_WIDTH, int SCR_HEIGHT)
{
	const auto hdrTextureSize{ hdrTextures.size() };
	unsigned int hdrFBO;
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glGenTextures(hdrTextureSize, hdrTextures.data());
	std::vector<unsigned int> attachments{};
	attachments.reserve(hdrTextureSize);
	for (std::size_t i{ 0 }; i < hdrTextures.size(); ++i)
	{
		glBindTexture(GL_TEXTURE_2D, hdrTextures[i]); // a Point Light, need to use a CUBEMAP!
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, hdrTextures[i], 0);
		attachments.emplace_back(GL_COLOR_ATTACHMENT0 + i);
	}
	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	glDrawBuffers(hdrTextureSize, attachments.data());

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Framebuffer not complete!\n";
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return { hdrFBO, RBO };
}

std::vector<unsigned int> getPingPongFBOs(int SCR_WIDTH, int SCR_HEIGHT)
{
	unsigned int pingpongFBO[2];
	unsigned int pingpongBuffer[2];
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongBuffer); // save the Gaussian Blured Result (one pass for each)
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0
		);
	}

	return { pingpongFBO[0], pingpongFBO[1], pingpongBuffer[0],pingpongBuffer[1] };
}

std::vector<unsigned int> getGbuffer(int SCR_WIDTH, int SCR_HEIGHT)
{
	unsigned int gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	unsigned int gPosition, gNormal, gAlbedoSpec;

	// - position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// - normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// - color + specular color buffer // RGBA -> (r, g, b, spec)
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

	// - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	unsigned int RBOdepth;
	glGenRenderbuffers(1, &RBOdepth);
	glBindRenderbuffer(GL_RENDERBUFFER, RBOdepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBOdepth);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Framebuffer not complete!\n";
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return { gBuffer, gPosition, gNormal, gAlbedoSpec, RBOdepth };
}

std::vector<glm::vec3> getSSAOKernel(int kernelSize)
{
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
	std::default_random_engine generator;
	std::vector<glm::vec3> ssaoKernel;
	ssaoKernel.reserve(kernelSize);
	float scale;
	for (unsigned int i{ 0 }; i < kernelSize; ++i)
	{
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0, // We vary the x and y direction in tangent space between -1.0 and 1.0
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) // and vary the z direction of the samples between 0.0 and 1.0  
									// (if we varied the z direction between -1.0 and 1.0 as well we'd have a sphere sample kernel)
		);
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		scale = (float)i / (float)kernelSize;
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;

		ssaoKernel.emplace_back(sample);
	}
	return ssaoKernel;
}

std::vector<glm::vec3> getSSAONoise(int kernelSize)
{
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
	std::default_random_engine generator;
	std::vector<glm::vec3> ssaoNoise;
	ssaoNoise.reserve(kernelSize);
	for (unsigned int i = 0; i < kernelSize; i++)
	{
		glm::vec3 noise(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			0.0f);
		ssaoNoise.emplace_back(noise);
	}
	
	return ssaoNoise;
}

unsigned int getSSAONoiseTex(const std::vector<glm::vec3>& ssaoNoise)
{
	unsigned int noiseTexture;
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, ssaoNoise.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return noiseTexture;
}

std::vector<unsigned int> getSSAO_FBO_Tex(int SCR_WIDTH, int SCR_HEIGHT)
{
	unsigned int ssaoFBO;
	glGenFramebuffers(1, &ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

	unsigned int ssaoColorBuffer;
	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);

	return { ssaoFBO, ssaoColorBuffer };
}

std::vector<unsigned int> getCaptureFBO_envCubeMap(int WIDTH, int HEIGHT)
{
	unsigned int captureFBO, captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, WIDTH, HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	unsigned int envCubemap;
	glGenTextures(1, &envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		// note that we store each face with 16 bit floating point values
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
			WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // for mipmap sampling
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, envCubemap, 0); // depthMap can do this, ColorTexture CAN NOT!

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Framebuffer not complete!\n";
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return { captureFBO, envCubemap, captureRBO };
}

void renderCube(unsigned int& cubeVAO, unsigned int& cubeVBO)
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			 // bottom face
			 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 // top face
			 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			  1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void renderQuad(unsigned int& quadVAO, unsigned int& quadVBO)
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void renderFloor(unsigned int& floorVAO, unsigned int& floorVBO)
{
	if(floorVAO == 0)
	{
		float planeVertices[] = {
			// positions            // normals         // texcoords
			 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
			-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  0.0f,   10.0f,
			-10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  0.0f,   0.0f,

			 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
			 10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  10.0f,
			-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  0.0f,   10.0f
		};
		glGenVertexArrays(1, &floorVAO);
		glGenBuffers(1, &floorVBO);
		glBindVertexArray(floorVAO);
		glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	}
	glBindVertexArray(floorVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void renderSphere(unsigned int& sphereVAO, unsigned int& indexCount)
{
	if (sphereVAO == 0)
	{
		glGenVertexArrays(1, &sphereVAO);

		unsigned int vbo, ebo;
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> uv;
		std::vector<glm::vec3> normals;
		std::vector<unsigned int> indices;

		const unsigned int X_SEGMENTS = 64;
		const unsigned int Y_SEGMENTS = 64;
		const float PI = 3.14159265359f;
		for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
		{
			for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
			{
				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y / (float)Y_SEGMENTS;
				float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
				float yPos = std::cos(ySegment * PI);
				float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

				positions.push_back(glm::vec3(xPos, yPos, zPos));
				uv.push_back(glm::vec2(xSegment, ySegment));
				normals.push_back(glm::vec3(xPos, yPos, zPos));
			}
		}

		bool oddRow = false;
		for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
		{
			if (!oddRow) // even rows: y == 0, y == 2; and so on
			{
				for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
				{
					indices.push_back(y * (X_SEGMENTS + 1) + x);
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				}
			}
			else
			{
				for (int x = X_SEGMENTS; x >= 0; --x)
				{
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
					indices.push_back(y * (X_SEGMENTS + 1) + x);
				}
			}
			oddRow = !oddRow;
		}
		indexCount = static_cast<unsigned int>(indices.size());

		std::vector<float> data;
		for (unsigned int i = 0; i < positions.size(); ++i)
		{
			data.push_back(positions[i].x);
			data.push_back(positions[i].y);
			data.push_back(positions[i].z);
			if (normals.size() > 0)
			{
				data.push_back(normals[i].x);
				data.push_back(normals[i].y);
				data.push_back(normals[i].z);
			}
			if (uv.size() > 0)
			{
				data.push_back(uv[i].x);
				data.push_back(uv[i].y);
			}
		}
		glBindVertexArray(sphereVAO);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
		unsigned int stride = (3 + 2 + 3) * sizeof(float);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
	}

	glBindVertexArray(sphereVAO);
	glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

// OpenGL 4.3
//GLenum glCheckError_(const char* file, int line)
//{
//	GLenum errorCode;
//	while ((errorCode = glGetError()) != GL_NO_ERROR)
//	{
//		std::string error;
//		switch (errorCode)
//		{
//		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
//		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
//		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
//		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
//		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
//		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
//		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
//		}
//		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
//	}
//	return errorCode;
//}

// debug mode debug <callback> Function
//void APIENTRY glDebugOutput(GLenum source,
//	GLenum type,
//	unsigned int id,
//	GLenum severity,
//	GLsizei length,
//	const char* message,
//	const void* userParam)
//{
//	// ignore non-significant error/warning codes
//	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;
//
//	std::cout << "---------------" << std::endl;
//	std::cout << "Debug message (" << id << "): " << message << std::endl;
//
//	switch (source)
//	{
//	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
//	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
//	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
//	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
//	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
//	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
//	} std::cout << std::endl;
//
//	switch (type)
//	{
//	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
//	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
//	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
//	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
//	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
//	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
//	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
//	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
//	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
//	} std::cout << std::endl;
//
//	switch (severity)
//	{
//	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
//	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
//	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
//	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
//	} std::cout << std::endl;
//	std::cout << std::endl;
//}