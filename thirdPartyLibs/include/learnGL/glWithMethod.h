#pragma once
#include <string>
#include <vector>
#include <glad/glad.h>
//#include <glad43/glad.h> // OpenGL 4.3 for debug feature
#include <GLFW/glfw3.h>
#include <learnGL/camera.h>
#include <learnGL/config.h>

#ifdef __MULTISAMPLE__
extern int MULTISAMPLER;
#endif

extern Camera g_myCamera;
extern double g_lastX;
extern double g_lastY;
extern float g_deltaTime;
extern bool g_firstMoveMouse;
extern int g_framebuffer_width;
extern int g_framebuffer_height;

void processInput(GLFWwindow* window);
void framebufferSizeCallBack(GLFWwindow* window, int width, int height);
void cursorCallbackClass(GLFWwindow* window, double xpos, double ypos);
void scrollCallbackClass(GLFWwindow* window, double xOffset, double yOffset);
unsigned int textureFromFile(const char* path, const std::string& directory, bool reverse = true, bool gamma = false);
unsigned int loadTexture(const char* path, bool reverse = true, GLenum format = GL_SRGB); // 3 or 4 channels texture
unsigned int loadNormalTex(const char* path, bool reverse = true, GLenum format = GL_RGB); // 3 channels normal texture
unsigned int loadParallaxTex(const char* path, bool reverse = true, GLenum format = GL_RED); // 1 channel texture (not only parallax)
unsigned int loadHDRadiance(const char* path);
unsigned int loadTransparentTex(const char* path, bool reverse = true);
unsigned int loadCubeMapTextures(const std::vector<std::string>& texture_faces);
std::string getFullPath(const std::string& relativePath);
std::vector<glm::mat4> getShadowCubeMatrices(const glm::vec3& lightPos, float fov = 90.0f, float aspect = 1.0f, float near_plane = 0.1f, float far_plane = 25.0f);
glm::vec3 calculateTangents(const glm::vec3& edge1, const glm::vec3& edge2, const glm::vec2& deltaUV1, const glm::vec2& deltaUV2);
std::vector<unsigned int> getFBO_Tex(int SCR_WIDTH = g_framebuffer_width, int SCR_HEIGHT = g_framebuffer_height);
std::vector<unsigned int> getMutisampleFBO_Tex(int sampler, int SCR_WIDTH = g_framebuffer_width, int SCR_HEIGHT = g_framebuffer_height);
std::pair<unsigned int, unsigned int> getDMsFBO_MAP(int SHADOW_WIDTH = 1024, int SHADOW_HEIGHT = 1024); // DM -- DepthMap's FBO [x] and Map [y]
std::pair<unsigned int, unsigned int> getDCMsFBO_MAP(int SHADOW_WIDTH = 1024, int SHADOW_HEIGHT = 1024); // DCM -- DepthCubeMap's FBO[x] and Map[y]
unsigned int getUBO(std::size_t structureSize);
std::vector<unsigned int> getHDR_FBO_Tex(int SCR_WIDTH = g_framebuffer_width, int SCR_HEIGHT = g_framebuffer_height);
std::vector<unsigned int> getHDR_FBO_MutipleTex(std::vector<unsigned int>& hdrTextures, int SCR_WIDTH = g_framebuffer_width, int SCR_HEIGHT = g_framebuffer_height);
std::vector<unsigned int> getPingPongFBOs(int SCR_WIDTH = g_framebuffer_width, int SCR_HEIGHT = g_framebuffer_height);
std::vector<unsigned int> getGbuffer(int SCR_WIDTH = g_framebuffer_width, int SCR_HEIGHT = g_framebuffer_height);
std::vector<unsigned int> getCaptureFBO_envCubeMap(int WIDTH = 512, int HEIGHT = 512);
void renderCube(unsigned int& cubeVAO, unsigned int& cubeVBO);
void renderQuad(unsigned int& quadVAO, unsigned int& quadVBO);
void renderFloor(unsigned int& floorVAO, unsigned int& floorVBO);
void renderSphere(unsigned int& sphereVAO, unsigned int& indexCount);
float lerp(float a, float b, float f);
std::vector<glm::vec3> getSSAOKernel(int kernelSize = 64);
std::vector<glm::vec3> getSSAONoise(int kernelSize = 16);
unsigned int getSSAONoiseTex(const std::vector<glm::vec3>& ssaoNoise);
std::vector<unsigned int> getSSAO_FBO_Tex(int SCR_WIDTH = g_framebuffer_width, int SCR_HEIGHT = g_framebuffer_height);
//GLenum glCheckError_(const char* file, int line);
//void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity,
//	GLsizei length, const char* message, const void* userParam);

#ifdef __MULTISAMPLE__
void updateFramebuffer(const unsigned int textureAtt, const unsigned int RBO, const bool isMultisample = false);
#endif

#ifndef __MULTISAMPLE__
void updateFramebuffer(const unsigned int textureAtt, const unsigned int RBO);
#endif

// a template should be writed in .h
// or you can write the definition in .tpp and #include the .tpp at the last of this file
// otherwise, it will break the original intention of generic programming, even cause linking error
template <typename T>
std::pair<unsigned int, unsigned int> getVAO_VBO(const std::vector<T>& your_vertexStruct)
{
	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(T) * your_vertexStruct.size(), your_vertexStruct.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(T), (void*)offsetof(T, aPosition));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(T), (void*)offsetof(T, aNormal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(T), (void*)offsetof(T, aTangent));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(T), (void*)offsetof(T, aTexcoords));
	glBindVertexArray(0);

	return { VAO,VBO };
}