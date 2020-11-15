#pragma once

#include "Mango/Core/Math.h"

#include "VertexArray.h"
#include "Texture.h"

namespace Mango {
	
	struct Material {
		Ref<Texture> AlbedoTexture;
		Ref<Texture> NormalTexture;
		Ref<Texture> RoughnessTexture;
		float3 AlbedoColor;
		float RoughnessValue;
		float Metalness;

		Material(const Ref<Texture>& albedoTex, const Ref<Texture>& normalTex, const Ref<Texture>& roughnessTex, const float3& color, float roughness, float metalness)
			: AlbedoTexture(albedoTex), NormalTexture(normalTex), RoughnessTexture(roughnessTex), AlbedoColor(color), RoughnessValue(roughness), Metalness(metalness)
		{}
	};

	struct Node {
		std::vector<Node> Children;
		std::vector<std::pair<Ref<VertexArray>, Ref<Material>>> Submeshes;
		xmmatrix Transform;

		Node(const xmmatrix& transform = XMMatrixIdentity())
			: Transform(transform)
		{}

		Node(const Node& other) = default;
	};

	enum MeshType {
		MeshType_Empty,
		MeshType_Cube,
		MeshType_Sphere,
		MeshType_Capsule,
		MeshType_Model
	};

	struct Mesh {
		Node RootNode;
		std::vector<Ref<Material>> Materials;
		MeshType Type = MeshType_Empty;
		std::string Path;

		Mesh() = default;
		Mesh(const Node& rootNode, MeshType type, const std::string& path = "")
			: RootNode(rootNode),
			Type(type),
			Path(path)
		{}

		static Ref<Mesh> CreateCube(const Ref<Material>& material);
		static Ref<Mesh> CreateSphere(const Ref<Material>& material, uint32_t xSegments = 32, uint32_t ySegments = 16);
		static Ref<Mesh> CreateCapsule(const Ref<Material>& material, uint32_t mantleSegments = 20, uint32_t ellipsoidSegments = 5);
		static Ref<Mesh> CreateModel(const std::vector<Ref<Material>>& material, TextureLibrary& textureLibrary, const std::string& file);
	};

	class MeshLibrary {
	public:
		MeshLibrary() = default;
		void Push(const std::string& name, const Ref<Mesh>& mesh);
		inline std::pair<std::string, Ref<Mesh>>& operator[](size_t index) { return mMeshes[index]; }
		inline size_t Size() const { return mMeshes.size(); }
		inline auto begin() { return mMeshes.begin(); }
		inline auto end() { return mMeshes.end(); }
	private:
		std::vector<std::pair<std::string, Ref<Mesh>>> mMeshes;
	};

}