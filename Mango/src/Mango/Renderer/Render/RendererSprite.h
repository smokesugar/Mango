#pragma once

#include <queue>

#include "Mango/Core/Math.h"
#include "Mango/Renderer/Texture.h"
#include "Mango/Renderer/Framebuffer.h"

namespace Mango {
	namespace Renderer {

		void InitSprite();
		void ShutdownSprite();
	
		void FlushSpriteQueue(std::queue<std::tuple<xmmatrix, xmmatrix, Ref<Texture2D>, float4>>& queue, const std::vector<Ref<ColorBuffer>>& rendertargets, const Ref<DepthBuffer>& depthbuffer);

	}
}