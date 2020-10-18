#pragma once

#include "Mango/Core/Math.h"
#include "Mango/Renderer/Camera.h"
#include "Mango/Renderer/Texture.h"
#include "Mango/Renderer/Mesh.h"

namespace Mango {
	
	struct TransformComponent {
		xmmatrix Transform = XMMatrixIdentity();

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const xmmatrix& mat)
			: Transform(mat) {}
	};

	struct TagComponent {
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {}
	};

	struct CameraComponent {
		Mango::Camera Camera;
		bool Primary = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(const Mango::Camera& cam)
			: Camera(cam) {}
	};

	struct SpriteRendererComponent {
		bool UsesTexture;
		float4 Color;
		Ref<Texture2D> Texture;

		SpriteRendererComponent() : Color(1.0f, 1.0f, 1.0f, 1.0f), UsesTexture(false) {}
		SpriteRendererComponent(const SpriteRendererComponent& other) = default;
		SpriteRendererComponent(const float4& color)
			: Color(color), UsesTexture(false)
		{}
		SpriteRendererComponent(const Ref<Texture2D>& texture)
			: Texture(texture), Color(0.0f, 0.0f, 0.0f, 1.0f), UsesTexture(true)
		{}
	};

	enum class MeshType {
		Empty, Cube
	};

	struct MeshComponent {
		Mango::Mesh Mesh;
		MeshType Type = MeshType::Empty;

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent(const Mango::Mesh& mesh, MeshType type)
			: Mesh(mesh), Type(type)
		{}
	};

}