#pragma once

#include "Mango.h"

namespace Mango {
	namespace ViewportInteraction {
		void Init();
		void Shutdown();
		ECS::Entity GetHoveredEntity(ECS::Registry& reg, const float2& viewportSize, const float2& mousePosition, const xmmatrix& viewProjection);
		void RenderSelectionOutline(ECS::Registry& reg, ECS::Entity entity, const xmmatrix& viewProjection, const Ref<Texture>& rendertarget);
	}
}