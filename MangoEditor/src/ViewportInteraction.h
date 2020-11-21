#pragma once

#include "Mango.h"

namespace Mango {
	namespace ViewportInteraction {
		void Init();
		void Shutdown();

		const Ref<Texture> GetRenderTarget();

		ECS::Entity GetHoveredEntity(const Ref<Scene>& scene, const float2& viewportSize, const float2& mousePosition, const xmmatrix& viewProjection);
		void RenderSelectionOutline(const Ref<Scene>& scene, ECS::Entity entity, const xmmatrix& viewProjection, const Ref<Texture>& rendertarget);
	}
}