#include "mgpch.h"
#include "Mango/Renderer/Mesh.h"
#include "Mango/Renderer/Render/Renderer.h"

#define CAPSULE_HEIGHT 2.0f
#define CAPSULE_RADIUS 0.5f

// Code adapted from https://github.com/LukasBanana/GeometronLib/blob/master/sources/MeshGeneratorCapsule.cpp

namespace Mango {

	struct Spherical {
		float Radius;
		float Theta;
		float Phi;

		Spherical(float radius, float theta, float phi)
			: Radius(radius), Theta(theta), Phi(phi) {}

		operator float3() {
			float x = Radius * cosf(Phi) * sinf(Theta);
			float y = Radius * sinf(Phi) * sinf(Theta);
			float z = Radius * cosf(Theta);

			return float3(x, y, z);
		}
	};

	Ref<Mesh> Mesh::CreateCapsule(const Ref<Material>& mat, uint32_t mantleSegments, uint32_t ellipsoidSegments) {
		const uint32_t segsHorz = Max(3u, mantleSegments);
		const uint32_t segsVert = 1u;
		const uint32_t segsV = Max(2u, ellipsoidSegments);

		const float invHorz = 1.0f / (float)segsHorz;
		const float invVert = 1.0f / (float)segsVert;
		const float invSegsV = 1.0f / (float)segsV;

		const float angleSteps = invHorz * TAU;
		const float halfHeight = (CAPSULE_HEIGHT - CAPSULE_RADIUS*2.0f)/2.0f;

		Spherical point(1.0f, 0.0f, 0.0f);
		float3 coord, normal;
		float2 texCoord;

		float angle = 0.0f;

		std::vector<float> vertices;

		for (uint32_t u = 0; u <= segsHorz; u++) {
			texCoord.x = sinf(angle);
			texCoord.y = cosf(angle);

			coord.x = texCoord.x * CAPSULE_RADIUS;
			coord.z = texCoord.y * CAPSULE_RADIUS;

			normal.x = texCoord.x;
			normal.y = 0;
			normal.z = texCoord.y;

			float len = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);

			normal.x /= len;
			normal.y /= len;
			normal.z /= len;

			texCoord.x = (float)(segsHorz - u) * invHorz;

			for(uint32_t v = 0; v <= segsVert; v++) {
				texCoord.y = (float)v * invVert;
				coord.y = Lerp(halfHeight, -halfHeight, texCoord.y);

				vertices.push_back(coord.x);
				vertices.push_back(coord.y);
				vertices.push_back(coord.z);
				vertices.push_back(normal.x);
				vertices.push_back(normal.y);
				vertices.push_back(normal.z);
				vertices.push_back(texCoord.x);
				vertices.push_back(texCoord.y);
			}

			angle += angleSteps;
		}

		const float coverSide[2] = {1.0f, -1.0f};
		size_t idxBaseOffsetEllipsoid[2] = { 0 };

		for (size_t i = 0; i < 2; i++)
		{
			idxBaseOffsetEllipsoid[i] = vertices.size() / 8;

			for (uint32_t v = 0; v <= segsV; v++)
			{
				texCoord.y = (float)v * invSegsV;
				point.Theta = texCoord.y * PI/2.0f;

				for (uint32_t u = 0; u <= segsHorz; u++) {
					texCoord.x = (float)u * invHorz;
					point.Phi = texCoord.x * TAU * coverSide[i] + PI/2.0f;

					coord = point;
					std::swap(coord.y, coord.z);

					coord.y *= coverSide[i];

					/* Get normal and move half-sphere */
					normal = coord;
					float len = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
					normal.x /= len;
					normal.y /= len;
					normal.z /= len;

					/* Transform coordinate with radius and height */
					coord.x *= CAPSULE_RADIUS;
					coord.y *= CAPSULE_RADIUS;
					coord.z *= CAPSULE_RADIUS;

					coord.y += halfHeight * coverSide[i];

					vertices.push_back(coord.x);
					vertices.push_back(coord.y);
					vertices.push_back(coord.z);
					vertices.push_back(normal.x);
					vertices.push_back(normal.y);
					vertices.push_back(normal.z);
					vertices.push_back(texCoord.x);
					vertices.push_back(texCoord.y);
				}
			}
		}

		auto idxOffset = 0;
		std::vector<uint16_t> indices;

		for (std::uint32_t u = 0; u < segsHorz; ++u)
		{
			for (std::uint32_t v = 0; v < segsVert; ++v)
			{
				auto i0 = idxOffset + v + 1 + segsVert;
				auto i1 = idxOffset + v;
				auto i2 = idxOffset + v + 1;
				auto i3 = idxOffset + v + 2 + segsVert;

				indices.push_back((uint16_t)i0);
				indices.push_back((uint16_t)i1);
				indices.push_back((uint16_t)i3);
				indices.push_back((uint16_t)i1);
				indices.push_back((uint16_t)i2);
				indices.push_back((uint16_t)i3);
			}

			idxOffset += (1 + segsVert);
		}

		/* Generate indices for the top and bottom */
		for (size_t i = 0; i < 2; ++i)
		{
			for (uint32_t v = 0; v < segsV; ++v)
			{
				for (uint32_t u = 0; u < segsHorz; ++u)
				{
					/* Compute indices for current face */
					auto i0 = idxBaseOffsetEllipsoid[i] + v * (segsHorz + 1u) + u;
					auto i1 = idxBaseOffsetEllipsoid[i] + v * (segsHorz + 1u) + (u + 1u);

					auto i2 = idxBaseOffsetEllipsoid[i] + (v + 1u) * (segsHorz + 1u) + (u + 1u);
					auto i3 = idxBaseOffsetEllipsoid[i] + (v + 1u) * (segsHorz + 1u) + u;

					indices.push_back((uint16_t)i0);
					indices.push_back((uint16_t)i1);
					indices.push_back((uint16_t)i3);
					indices.push_back((uint16_t)i1);
					indices.push_back((uint16_t)i2);
					indices.push_back((uint16_t)i3);
				}
			}
		}

		auto vb = Ref<VertexBuffer>(VertexBuffer::Create(vertices.data(), vertices.size() / 8, 8 * sizeof(float)));
		auto ib = Ref<IndexBuffer>(IndexBuffer::Create(indices.data(), indices.size()));

		Node node;
		node.Submeshes.push_back({ CreateRef<VertexArray>(vb, ib), mat });
		Ref<Mesh> mesh = CreateRef<Mesh>(node, MeshType_Capsule);
		mesh->Materials.push_back(mat);

		return mesh;
	}

}