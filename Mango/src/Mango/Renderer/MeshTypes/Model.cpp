#include "mgpch.h"
#include "Mango/Renderer/Mesh.h"
#include "Mango/Renderer/Render/Renderer.h"
#include "Mango/Core/Application.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>

namespace Mango {

	static std::vector<Ref<Material>> sMaterials;

	static float3 globalAabbMin;
	static float3 globalAabbMax;

	static Ref<VertexArray> ProcessMesh(aiMesh* mesh, const aiScene* scene, const xmmatrix& transform, BoundingBox& aabb)
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

			aabb.Min = Min(aabb.Min, float3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
			aabb.Max = Max(aabb.Max, float3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));

			xmvector transformed = XMVector4Transform({ mesh->mVertices[i].x ,mesh->mVertices[i].y,mesh->mVertices[i].z, 1.0f }, transform);
			float3 transformedF; XMStoreFloat3(&transformedF, transformed);
			globalAabbMin = Min(globalAabbMin, transformedF);
			globalAabbMax = Max(globalAabbMax, transformedF);

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

	static Node ProcessNode(aiNode* ainode, const aiScene* scene, const xmmatrix& parentTransform)
	{
		float4x4 transform = *(float4x4*)&ainode->mTransformation;
		Node node(XMMatrixTranspose(XMLoadFloat4x4(&transform)));

		xmmatrix accumulatedTransform = node.Transform * parentTransform;

		for (size_t i = 0; i < ainode->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[ainode->mMeshes[i]];
			BoundingBox aabb(float3(INFINITY, INFINITY, INFINITY), float3(-INFINITY, -INFINITY, -INFINITY));
			Ref<VertexArray> va = ProcessMesh(mesh, scene, accumulatedTransform, aabb);
			node.Submeshes.push_back({ va, aabb, sMaterials[mesh->mMaterialIndex] });
		}

		for (size_t i = 0; i < ainode->mNumChildren; i++) {
			node.Children.push_back(ProcessNode(ainode->mChildren[i], scene, accumulatedTransform));
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

	Ref<Mesh> Mesh::CreateModel(const std::vector<Ref<Material>>& materials, TextureLibrary& textureLibrary, const std::string& file)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices | aiProcess_ValidateDataStructure);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::stringstream ss;
			ss << "Failed to load 3D model: " << file << std::endl;
			Application::Get().GetRuntimeLog().AddLog(ss.str());
			MG_CORE_ERROR("Unable to load 3D model '{0}'.", file);
			importer.FreeScene();
			return CreateRef<Mesh>();
		}

		std::string directory = file.substr(0, file.find_last_of('\\'));
		directory.append("\\");

		sMaterials = materials;
		if (sMaterials.empty()) {
			for (uint32_t i = 0; i < scene->mNumMaterials; i++)
			{	
				aiMaterial* material = scene->mMaterials[i];
				std::string path;

				Ref<Texture> albedoTexture = GetTexturePath(path, aiTextureType_DIFFUSE, material) ? textureLibrary.Get(directory + path, Format::RGBA8_UNORM_SRGB, Texture_Trilinear) : Renderer::GetWhiteTexture();
				Ref<Texture> normalTexture = GetTexturePath(path, aiTextureType_NORMALS, material) ? textureLibrary.Get(directory + path, Format::RGBA8_UNORM, Texture_Trilinear) : nullptr;
				Ref<Texture> roughnessTexture = GetTexturePath(path, aiTextureType_UNKNOWN, material) ? textureLibrary.Get(directory + path, Format::RGBA8_UNORM, Texture_Trilinear) : Renderer::GetWhiteTexture();

				float3 albedoColor;
				if (albedoTexture == Renderer::GetWhiteTexture())
					material->Get(AI_MATKEY_COLOR_DIFFUSE, *(aiColor3D*)&albedoColor);
				else
					albedoColor = float3(1.0f, 1.0f, 1.0f);

				float roughness;
				if (roughnessTexture == Renderer::GetWhiteTexture()) {
					if (material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, roughness) == aiReturn_FAILURE)
						roughness = 0.5f;
				}
				else
					roughness = 1.0f;
				
				float metalness;
				if (material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, metalness) == aiReturn_FAILURE)
					metalness = 0.1f;

				sMaterials.push_back(CreateRef<Material>(albedoTexture, normalTexture, roughnessTexture, albedoColor, roughness, metalness));
			}
		}


		globalAabbMin = float3(INFINITY, INFINITY, INFINITY);
		globalAabbMax = float3(-INFINITY, -INFINITY, -INFINITY);

		Node node = ProcessNode(scene->mRootNode, scene, XMMatrixIdentity());
		Ref<Mesh> mesh = CreateRef<Mesh>(node, MeshType_Model, BoundingBox(globalAabbMin, globalAabbMax), file);
		mesh->Materials = sMaterials;

		importer.FreeScene();
		sMaterials.clear();

		return mesh;
	}

}