#include "mgpch.h"
#include "Mango/Renderer/Mesh.h"
#include "Mango/Renderer/Render/Renderer.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Mango {

	static std::vector<Ref<Material>> sMaterials;

	static Ref<VertexArray> ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<float> vertices;
		vertices.reserve(mesh->mNumVertices * 8);
		std::vector<uint16_t> indices;
		indices.reserve(mesh->mNumFaces*3);

		for (uint32_t i = 0; i < mesh->mNumVertices; i++)
		{
			vertices.push_back(mesh->mVertices[i].x);
			vertices.push_back(mesh->mVertices[i].y);
			vertices.push_back(mesh->mVertices[i].z);
			vertices.push_back(mesh->mNormals[i].x);
			vertices.push_back(mesh->mNormals[i].y);
			vertices.push_back(mesh->mNormals[i].z);
			
			if (mesh->mTextureCoords[0]) {
				vertices.push_back(mesh->mTextureCoords[0][i].x);
				vertices.push_back(mesh->mTextureCoords[0][i].y);
			}
			else {
				vertices.push_back(0.0f);
				vertices.push_back(0.0f);
			}
		}

		for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		return CreateRef<VertexArray>(Ref<VertexBuffer>(VertexBuffer::Create(vertices.data(), vertices.size()/8, 8*sizeof(float))), Ref<IndexBuffer>(IndexBuffer::Create(indices.data(), indices.size())));
	}

	static Node ProcessNode(aiNode* ainode, const aiScene* scene)
	{
		float4x4 transform = *(float4x4*)&ainode->mTransformation;
		Node node(XMLoadFloat4x4(&transform));

		for (size_t i = 0; i < ainode->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[ainode->mMeshes[i]];
			node.Submeshes.push_back({ ProcessMesh(mesh, scene), sMaterials[mesh->mMaterialIndex] });
		}

		for (size_t i = 0; i < ainode->mNumChildren; i++) {
			node.Children.push_back(ProcessNode(ainode->mChildren[i], scene));
		}

		return node;
	}

	bool GetTexturePath(std::string& path, aiTextureType type, aiMaterial* mat) {
		if (mat->GetTextureCount(type) > 0) {
			aiString str;
			mat->GetTexture(type, 0, &str);
			path = str.C_Str();
			return true;
		}
		else
			return false;
	}

	Mesh Mesh::CreateModel(const std::vector<Ref<Material>>& materials, TextureLibrary& textureLibrary, const std::string& file)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
		MG_CORE_ASSERT(scene && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && scene->mRootNode, "Failed to load 3D model '{0}'.", file);

		std::string directory = file.substr(0, file.find_last_of('\\'));
		directory.append("\\");

		sMaterials = materials;
		if (sMaterials.empty()) {
			for (uint32_t i = 0; i < scene->mNumMaterials; i++)
			{	
				aiMaterial* material = scene->mMaterials[i];
				std::string path;

				Ref<Texture2D> albedoTexture = GetTexturePath(path, aiTextureType_DIFFUSE, material) ? textureLibrary.Get(directory + path, Format::RGBA8_UNORM_SRGB) : Renderer::GetWhiteTexture();
				Ref<Texture2D> normalTexture = GetTexturePath(path, aiTextureType_NORMALS, material) ? textureLibrary.Get(directory + path, Format::RGBA8_UNORM) : nullptr;
				Ref<Texture2D> roughnessTexture = GetTexturePath(path, aiTextureType_SPECULAR, material) ? textureLibrary.Get(directory + path, Format::RGBA8_UNORM) : Renderer::GetWhiteTexture();

				float3 albedoColor;
				if (albedoTexture == Renderer::GetWhiteTexture())
					material->Get(AI_MATKEY_COLOR_DIFFUSE, *(aiColor3D*)&albedoColor);
				else
					albedoColor = float3(1.0f, 1.0f, 1.0f);
				
				float roughness = roughnessTexture == Renderer::GetWhiteTexture() ? 0.5f : 1.0f;
				float metalness = 0.1f;

				sMaterials.push_back(CreateRef<Material>(albedoTexture, normalTexture, roughnessTexture, albedoColor, roughness, metalness));
			}
		}

		Mesh mesh(ProcessNode(scene->mRootNode, scene));
		mesh.Materials = sMaterials;

		importer.FreeScene();
		sMaterials.clear();

		return mesh;
	}

}