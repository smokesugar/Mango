#include "mgpch.h"
#include "Mango/Renderer/Mesh.h"
#include "Mango/Renderer/Render/Renderer.h"

#include "Mango/Core/Math.h"

namespace Mango {

	// Code straight up stolen from https://github.com/JoeyDeVries/Cell/blob/56726959fa85b4c087f9e87cc67d504db32ea41e/cell/mesh/sphere.cpp
	
	Mesh Mesh::CreateSphere(const Ref<Material>& mat, uint32_t xSegments, uint32_t ySegments)
	{
		std::vector<float> vertices;
		for (uint32_t y = 0; y <= ySegments; y++)
		{
			for (uint32_t x = 0; x <= xSegments; x++)
			{
				float xSegment = (float)x / (float)xSegments;
				float ySegment = (float)y / (float)ySegments;
				float xPos = std::cos(xSegment * TAU) * std::sin(ySegment * PI); // TAU is 2PI
				float yPos = std::cos(ySegment * PI);
				float zPos = std::sin(xSegment * TAU) * std::sin(ySegment * PI);

				float len = sqrtf(xPos*xPos + yPos*yPos + zPos*zPos);

				xPos /= len;
				yPos /= len;
				zPos /= len;

				vertices.push_back(xPos / 2.0f);
				vertices.push_back(yPos / 2.0f);
				vertices.push_back(zPos / 2.0f);

				vertices.push_back(xPos);
				vertices.push_back(yPos);
				vertices.push_back(zPos);

				vertices.push_back(xSegment);
				vertices.push_back(ySegment);
			}
		}

		std::vector<uint16_t> indices;
		for (uint16_t y = 0; y < ySegments; ++y)
		{
			for (uint16_t x = 0; x < xSegments; ++x)
			{
				indices.push_back((y + 1) * (xSegments + 1) + x);
				indices.push_back(y * (xSegments + 1) + x);
				indices.push_back(y * (xSegments + 1) + x + 1);

				indices.push_back((y + 1) * (xSegments + 1) + x);
				indices.push_back(y * (xSegments + 1) + x + 1);
				indices.push_back((y + 1) * (xSegments + 1) + x + 1);
			}
		}

		auto vb = Ref<VertexBuffer>(VertexBuffer::Create(vertices.data(), vertices.size()/8, 8 * sizeof(float)));
		auto ib = Ref<IndexBuffer>(IndexBuffer::Create(indices.data(), indices.size()));

		Node node;
		node.Submeshes.push_back({ CreateRef<VertexArray>(vb, ib), mat });

		Mesh mesh(node);
		mesh.Materials.push_back(mat);

		return mesh;
	}

}