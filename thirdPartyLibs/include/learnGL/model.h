#pragma once
#include <string>
#include <vector>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnGL/shader.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

constexpr int MAX_BONE_INFLUENCE{ 4 };

struct Vertex
{
	glm::vec3 m_position{};
	glm::vec3 m_normal{};
	glm::vec2 m_texCoord{};
	// [ -0.5f, -0.5f, -0.5f,|  0.0f,  0.0f, -1.0f,|  1.0f, 0.0f ]
	glm::vec3 m_tangent{};
	glm::vec3 m_bitangent{};
	//bone indexes which will influence this vertex
	std::array<int, MAX_BONE_INFLUENCE> m_boneIDs{};
	//weights from each bone
	std::array<float, MAX_BONE_INFLUENCE> m_weights{};

};

struct Texture
{
	unsigned int m_id{};
	std::string m_type{};
	std::string m_path{};

	Texture() = default;
	Texture(unsigned int id, const std::string& type, const std::string& path)
		: m_id{id}, m_type{type}, m_path{path}
	{}
};

class Mesh
{
public:
	explicit Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	unsigned int getMeshVAO() const { return m_VAO; }
	
	const std::vector<Vertex>& getVertices() const { return m_vertices; }
	const std::vector<unsigned int>& getIndices() const { return m_indices; }
	const std::vector<Texture>& getTextures() const { return m_textures; }
	std::vector<Vertex>& getVertices() { return m_vertices; }
	std::vector<unsigned int>& getIndices() { return m_indices; }
	std::vector<Texture>& getTextures() { return m_textures; }
	
	void draw(const Shader& shader, const unsigned int amount = 0) const;
	void nativeDraw(const Shader& shader, const unsigned int amount) const;

private:
	std::vector<Vertex> m_vertices{};
	std::vector<unsigned int> m_indices{};
	std::vector<Texture> m_textures{};
	unsigned int m_VAO;
	unsigned int m_VBO;
	unsigned int m_EBO;
	
	void setUpMesh();
};

class Model
{
public:
	explicit Model(const char* path, bool normalize = false, bool gamma = false);

	const std::vector<Mesh>& getMeshes() const { return m_meshes; }
	void drawModel(const Shader& shader, const unsigned int amount = 0) const;
	void drawModel_Native(const Shader& shader, const unsigned int amount = 0) const;
private:
	std::vector<Mesh> m_meshes{};
	std::string m_directory{};
	bool m_gammaCorrection{};
	bool m_normalize{};

	void loadModel(const std::string& path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene, bool normalize);
	std::vector<Texture> loadMeterialTextures(aiMaterial* material, aiTextureType type, std::string typeName);
};

