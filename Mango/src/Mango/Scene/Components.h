#pragma once

#include "Mango/Core/Math.h"
#include "Mango/Renderer/Camera.h"

namespace Mango {
	
	struct TransformComponent {
		xmmatrix Transform;

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
		Ref<Mango::Camera> Camera;
		bool Primary = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(const Ref<Mango::Camera>& cam)
			: Camera(cam) {}
	};

	struct SpriteRendererComponent {
		float4 Color;

		SpriteRendererComponent() : Color(1.0f, 1.0f, 1.0f, 1.0f) {}
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const float4& color)
			: Color(color)
		{}
	};

}