#pragma once

#include "Mango/Core/Math.h"

#include "VertexArray.h"

namespace Mango {
	
	struct Node {
		std::vector<Node> Children;
		std::vector<Ref<VertexArray>> Submeshes;
		xmmatrix Transform;

		Node(const xmmatrix& transform = XMMatrixIdentity())
			: Transform(transform)
		{}

		Node(const Node& other) = default;
	};

	struct Mesh {
		Node RootNode;

		Mesh() = default;
		Mesh(const Node& rootNode)
			: RootNode(rootNode)
		{}

		static Mesh CreateCube();
		static Mesh CreateSphere(uint32_t xSegments = 32, uint32_t ySegments = 16);
		static Mesh CreateCapsule(uint32_t mantleSegments = 20, uint32_t ellipsoidSegments = 5);
		static Mesh CreateModel(const std::string& file);
	};

}