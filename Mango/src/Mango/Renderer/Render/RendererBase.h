#pragma once

#include "Mango/Core/Math.h"
#include "Mango/Renderer/Texture.h"

namespace Mango {

	static constexpr float4 RENDERER_CLEAR_COLOR = float4(1.0f, 0.0f, 1.0f, 1.0f);

	namespace Renderer {

		void InitBase();
		void ShutdownBase();
		void DrawScreenQuad();

		SamplerState& LinearSampler();
		SamplerState& LinearSamplerClamp();
		SamplerState& PointSampler();

		void StoreViewMatrix(const xmmatrix& view);
		void StoreProjectionMatrix(const xmmatrix& projection);
		void StoreJitterMatrix(const xmmatrix& jitter);
		const xmmatrix& GetViewMatrix();
		const xmmatrix& GetProjectionMatrix();
		const xmmatrix& GetPrevViewMatrix();
		const xmmatrix& GetPrevProjectionMatrix();
		const xmmatrix& GetJitterMatrix();
	}
}