#pragma once

#include "Mango/Renderer/RenderTarget.h"
#include "Mango/Renderer/Mesh.h"

#define SKYBOX_RESOLUTION 2048

namespace Mango {
	namespace Renderer {
		
		void InitLighting();
		void ShutdownLighting();

		void ShadowmapPass(std::unordered_map<Ref<Material>, std::vector<std::tuple<Ref<VertexArray>, xmmatrix, xmmatrix>>>& queue);
		void LightingPass(const Ref<Texture>& color, const Ref<Texture>& normal, const Ref<Texture>& ao, const Ref<DepthBuffer>& depth, const Ref<Texture>& rendertarget);

		void SubmitDirectionalLight(const float3& direction, const float3& color);
		void SubmitPointLight(const float3& position, const float3& color);

		void SetSkybox(const Ref<Cubemap>& skybox);
		const Ref<Cubemap>& GetSkybox();

		float& EnvironmentStrength();

		void InitializeCubemap(const Ref<Cubemap>& cubemap);
	}
}