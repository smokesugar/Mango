#pragma once

#include <queue>

#include "Mango/Core/Math.h"
#include "Mango/Renderer/Texture.h"
#include "Mango/Renderer/RenderTarget.h"
#include "Mango/Renderer/VertexArray.h"

namespace Mango {
	namespace Renderer {

		void InitSprite();
		void ShutdownSprite();

		const Ref<VertexArray>& GetSpriteQuadVertexArray();
	
		void FlushSpriteQueue(std::queue<std::tuple<xmmatrix, xmmatrix, Ref<Texture>, float4>>& queue, const std::vector<Ref<Texture>>& rendertargets, const Ref<DepthBuffer>& depthbuffer);

	}
}