#pragma once

#include "Mango/Core/Math.h"

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

}