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

	enum class MeshType {
		Empty, Cube
	};

	struct Mesh {
		Node RootNode;
		MeshType Type;

		Mesh()
			: Type(MeshType::Empty)
		{}

		Mesh(const Node& rootNode, MeshType type)
			: RootNode(rootNode), Type(type)
		{}

		static Mesh CreateCube();
	};

}