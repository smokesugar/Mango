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
	};

}