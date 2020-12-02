#pragma once

#include "Mango/Renderer/Mesh.h"
#include "Mango/Renderer/RenderTarget.h"

namespace Mango {
	namespace Renderer {

		void InitGeometry();
		void ShutdownGeometry();
		
		void RenderToGBuffer(std::unordered_map<Ref<Material>, std::vector<std::tuple<Ref<VertexArray>, BoundingBox, xmmatrix, xmmatrix>>>& queue, const std::vector<Ref<Texture>>& rendertargets, const Ref<DepthBuffer>& depthBuffer);

	}
}