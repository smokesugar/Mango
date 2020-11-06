#pragma once

#include "Mango/Renderer/Framebuffer.h"
#include "Mango/Renderer/Mesh.h"

namespace Mango {
	namespace Renderer {
		
		void InitLighting();
		void ShutdownLighting();

		void ShadowmapPass(std::unordered_map<Ref<Material>, std::vector<std::tuple<Ref<VertexArray>, xmmatrix, xmmatrix>>>& queue);
		void LightingPass(const Ref<ColorBuffer>& color, const Ref<ColorBuffer>& normal, const Ref<DepthBuffer>& depth, const Ref<ColorBuffer>& rendertarget);

		void SubmitDirectionalLight(const float3& direction, const float3& color);
		void SubmitPointLight(const float3& position, const float3& color);

	}
}