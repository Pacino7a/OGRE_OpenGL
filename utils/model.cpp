#include <learnGL/model.h>
#include <learnGL/glWithMethod.h>
#include <map>

std::vector<Texture> g_loadedTextures;

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
	: m_vertices{ vertices }, m_indices{ indices }, m_textures{ textures }
{
	setUpMesh();
}

void Mesh::setUpMesh()
{
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);

	// Activate Vertices' attributes in VBO
	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_normal));
	// vertex texCoords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_texCoord));
	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_tangent));
	// vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_bitangent));
	// vertex boneIDs
	glEnableVertexAttribArray(5);
	glVertexAttribIPointer(5,  4,  GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_boneIDs));
	// vertex weight from each bone
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_weights));

	glBindVertexArray(0);
}

void Mesh::draw(const Shader& shader, const unsigned int amount) const
{
	unsigned int diffuseNr{ 1 };
	unsigned int specularNr{ 1 };
	unsigned int normalNr{ 1 };
	unsigned int heightNr{ 1 };
	unsigned int roughnessNr{ 1 };
	shader.use();
	for (int i{ 0 }; i < m_textures.size(); ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i); // activate Texture Unit[i]
		std::string type{ m_textures[i].m_type };
		std::string number;
		if (type == "texture_diffuse")
		{
			//std::cout << "Diffuse\n";
			number = std::to_string(diffuseNr++);
		}
		else if (type == "texture_specular")
		{
			//std::cout << "Specular\n";
			number = std::to_string(specularNr++);
		}
		else if (type == "texture_normal")
		{
			//std::cout << "Normal\n";
			number = std::to_string(normalNr++);
		}
		else if (type == "texture_height")
		{
			//std::cout << "Height\n";
			number = std::to_string(heightNr++);
		}
		else if (type == "texture_roughness")
		{
			//std::cout << "Roughness\n";
			number = std::to_string(roughnessNr++);
		}

		shader.setInt(("material." + type + number).c_str(), i); // inject [i] (TexUnit[i]) to the shader
		glBindTexture(GL_TEXTURE_2D, m_textures[i].m_id); // bind texture's data to TexUnit[i] (require Active)
	}

	glBindVertexArray(m_VAO);
	if(!amount)
		glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
	else
		glDrawElementsInstanced(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0, amount);
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0); // reset Active to avoid Unexpected Changes.
}

void Mesh::nativeDraw(const Shader& shader, const unsigned int amount) const
{
	shader.use();
	glBindVertexArray(m_VAO);
	if (!amount)
		glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
	else
		glDrawElementsInstanced(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0, amount);
	glBindVertexArray(0);
}


Model::Model(const char* path, bool normalize, bool gamma)
	: m_gammaCorrection{ gamma }, m_normalize{normalize}
{
	loadModel(path);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	//for (unsigned int i{ 0 }; i < scene->mNumMeshes; ++i) // directly push
	//{
	//	m_meshes.emplace_back(processMesh(scene->mMeshes[i], scene));
	//}
	for (unsigned int i{ 0 }; i < node->mNumMeshes; ++i) // extra control over your mesh data (parent/child)
	{
		aiMesh* mesh{ scene->mMeshes[node->mMeshes[i]] }; // node returns mesh's index and use this to index the REAL Mesh array in Scene Obj
		m_meshes.emplace_back(processMesh(mesh, scene, m_normalize));  // get node's mesh
	}
	for (unsigned int i{ 0 }; i < node->mNumChildren; ++i) // traversal all nodes
	{
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene, bool normalize) // extract every mesh's properties
{
	// a model consists of meshes (a Car Model)
	// mesh is a small entity (a Engine Mesh, a Steering wheel Mesh, ...)
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	// extract current mesh's position, normal and texCoord
	for (unsigned int i{ 0 }; i < mesh->mNumVertices; ++i)
	{
		Vertex vertex;
		vertex.m_position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z); // mVertice is the position Array
		vertex.m_normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z); // mNormals
		if (mesh->mTextureCoords[0]) // aiMesh can hold up to 8 different texture Coordinates per vertex. We only extract the first
		{
			vertex.m_texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}
		else
			vertex.m_texCoord = glm::vec2(0.0f, 0.0f);
		if (mesh->HasTangentsAndBitangents())
		{
			vertex.m_tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
			vertex.m_bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
		}
		else
		{
			std::cout << "This mash has NO TB!\n";
			vertex.m_tangent = glm::vec3(1.0f, 0.0f, 0.0f);
			vertex.m_bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
		}
		vertex.m_boneIDs = std::array<int, 4>{0};
		vertex.m_weights = std::array<float, 4>{0};
		vertices.emplace_back(std::move(vertex));
	}
	// extract indices of the mesh
	for (unsigned int i{ 0 }; i < mesh->mNumFaces; ++i) // a Mesh consists of Many Faces, so we extract each face's indices of the mesh
	{
		aiFace face{ mesh->mFaces[i] };
		for (unsigned int j{ 0 }; j < face.mNumIndices; ++j)
			indices.emplace_back(face.mIndices[j]);
	}
	// extract material(texture) Units of the mesh
	if (mesh->mMaterialIndex >= 0) // mesh only holds indices to a Material Obj. Use `scene` to index Materials
	{
		aiMaterial* material{ scene->mMaterials[mesh->mMaterialIndex] };
		auto diffuseMap{ loadMeterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse") };
		textures.insert(textures.end(), diffuseMap.begin(), diffuseMap.end());
		auto specularMap{ loadMeterialTextures(material, aiTextureType_SPECULAR, "texture_specular") };
		textures.insert(textures.end(), specularMap.begin(), specularMap.end());
		auto normalMap{ loadMeterialTextures(material, aiTextureType_NORMALS, "texture_normal") };
		textures.insert(textures.end(), normalMap.begin(), normalMap.end());
		auto heightMap{ loadMeterialTextures(material, aiTextureType_HEIGHT, "texture_height") };
		textures.insert(textures.end(), heightMap.begin(), heightMap.end());
		auto roughnessMap{ loadMeterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness") };
		textures.insert(textures.end(), heightMap.begin(), heightMap.end());
	}
	// extract bones (Bone-centered)
	std::map<std::string, int> boneNameToID;
	std::vector<glm::mat4> boneOffsets; 
	for (unsigned int boneIdx = 0, boneCount = 0; boneIdx < mesh->mNumBones; ++boneIdx, ++boneCount)
	{
		aiBone* bone = mesh->mBones[boneIdx];
		std::string boneName(bone->mName.C_Str());

		int boneID = 0;
		if (!boneNameToID.contains(boneName))
		{
			boneID = boneCount;
			boneNameToID[boneName] = boneID;
			aiMatrix4x4 a = bone->mOffsetMatrix;
			glm::mat4 gmat(
				a.a1, a.b1, a.c1, a.d1,
				a.a2, a.b2, a.c2, a.d2,
				a.a3, a.b3, a.c3, a.d3,
				a.a4, a.b4, a.c4, a.d4
			);
			boneOffsets.emplace_back(gmat);
		}
		else
		{
			boneID = boneNameToID[boneName];
		}
		
		for (unsigned int weightIdx = 0; weightIdx < bone->mNumWeights; ++weightIdx)
		{
			unsigned int vertexID = bone->mWeights[weightIdx].mVertexId;
			float weight = bone->mWeights[weightIdx].mWeight;

			for (int i = 0; i < 4; ++i)
			{
				if (vertices[vertexID].m_weights[i] == 0.0f)
				{
					vertices[vertexID].m_boneIDs[i] = boneID;
					vertices[vertexID].m_weights[i] = weight;
					break;
				}
			}
		}
	}

	// Normalize Vertices' Position
	if(normalize)
	{
		glm::vec3 minPos(FLT_MAX);
		glm::vec3 maxPos(-FLT_MAX);

		for (const auto& v : vertices)
		{
			minPos = glm::min(minPos, v.m_position);
			maxPos = glm::max(maxPos, v.m_position);
		}

		glm::vec3 center = (minPos + maxPos) * 0.5f;
		glm::vec3 size = maxPos - minPos;
		float maxLength = std::max({ size.x, size.y, size.z });

		for (auto& v : vertices)
		{
			v.m_position = (v.m_position - center) / maxLength;
		}
	}

	return Mesh{ vertices,indices,textures };
}

std::vector<Texture> Model::loadMeterialTextures(aiMaterial* material, aiTextureType type, std::string typeName)
{
	std::vector<Texture> textures{};
	for (unsigned int i{ 0 }; i < material->GetTextureCount(type); ++i)
	{
		aiString str;
		material->GetTexture(type, i, &str);
		bool skip{ false };
		for (const auto& loadedTexture : g_loadedTextures)
		{
			if (std::strcmp(loadedTexture.m_path.data(), str.C_Str()) == 0)
			{
				textures.emplace_back(loadedTexture);
				skip = true;
				break;
			}
		}
		if (!skip) // if we have loaded the texture before, we skip to Reload it.
		{
			Texture texture;
			texture.m_id = textureFromFile(str.C_Str(), m_directory);
			texture.m_type = typeName;
			texture.m_path = str.C_Str();
			textures.emplace_back(texture);
			g_loadedTextures.emplace_back(texture);
		}
	}
	return textures;
}

void Model::loadModel(const std::string& path)
{
	Assimp::Importer importer;
	const aiScene* scene{ importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace ) };
	if (!scene || scene->mFlags && AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << '\n';
		return;
	}
	m_directory = path.substr(0, path.find_last_of('/'));
	std::cout << "Model_Path: " << path << '\n' << "Model_Dir: " << m_directory << '\n';
	processNode(scene->mRootNode, scene);
}

void Model::drawModel(const Shader& shader, const unsigned int amount) const
{
	for (const auto& mesh : m_meshes)
		mesh.draw(shader, amount);
}

void Model::drawModel_Native(const Shader& shader, const unsigned int amount) const
{
	for (const auto& mesh : m_meshes)
		mesh.nativeDraw(shader, amount);
}