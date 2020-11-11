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

	struct Mesh {
		Node RootNode;
		std::vector<Ref<Material>> Materials;

		Mesh() = default;
		Mesh(const Node& rootNode)
			: RootNode(rootNode)
		{}

		static Mesh CreateCube(const Ref<Material>& material);
		static Mesh CreateSphere(const Ref<Material>& material, uint32_t xSegments = 32, uint32_t ySegments = 16);
		static Mesh CreateCapsule(const Ref<Material>& material, uint32_t mantleSegments = 20, uint32_t ellipsoidSegments = 5);
		static Mesh CreateModel(const std::vector<Ref<Material>>& material, TextureLibrary& textureLibrary, const std::string& file);
	};

}