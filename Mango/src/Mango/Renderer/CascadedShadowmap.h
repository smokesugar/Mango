#pragma once

#include <cinttypes>

#include "Mango/Core/Math.h"

namespace Mango {

	class CascadedShadowmap {
	public:
		virtual ~CascadedShadowmap() {}

		virtual void Clear(float value) = 0;

		virtual void BindAsRenderTarget() const = 0;
		virtual void BindAsTexture(size_t slot) const = 0;

		static std::vector<float> GenerateCascadeDistances(const xmmatrix& projection, uint32_t numCascades);

		static std::vector<xmmatrix> GenerateMatrices(const float3& direction, const xmmatrix& view, const xmmatrix& projection, uint32_t numCascades, uint32_t textureSize);

		static CascadedShadowmap* Create(uint32_t width, uint32_t height, uint32_t cascades);
	};

}