#pragma once

#include "Mango.h"

namespace Mango {
	namespace MousePicker {
		void Init();
		void Shutdown();
		ECS::Entity GetHoveredEntity(ECS::Registry& reg, const float2& viewportSize, const float2& mousePosition, const xmmatrix& viewProjection);
	}
}