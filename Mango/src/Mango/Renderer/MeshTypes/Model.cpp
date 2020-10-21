#include "mgpch.h"
#include "Mango/Renderer/Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Mango {

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
			node.Submeshes.push_back(ProcessMesh(mesh, scene));
		}

		for (size_t i = 0; i < ainode->mNumChildren; i++) {
			node.Children.push_back(ProcessNode(ainode->mChildren[i], scene));
		}

		return node;
	}

	Mesh Mesh::CreateModel(const std::string& file)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
		MG_CORE_ASSERT(scene && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && scene->mRootNode, "Failed to load 3D model '{0}'.", file);
		Mesh mesh(ProcessNode(scene->mRootNode, scene));
		importer.FreeScene();
		return mesh;
	}

}