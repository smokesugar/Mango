#pragma once

#include "Mango/Renderer/Mesh.h"
#include "Mango/Renderer/Framebuffer.h"

namespace Mango {
	namespace Renderer {

		void InitGeometry();
		void ShutdownGeometry();
		
		void RenderToGBuffer(std::unordered_map<Ref<Material>, std::vector<std::tuple<Ref<VertexArray>, xmmatrix, xmmatrix>>>& queue, const std::vector<Ref<ColorBuffer>>& rendertargets, const Ref<DepthBuffer>& depthBuffer);

	}
}