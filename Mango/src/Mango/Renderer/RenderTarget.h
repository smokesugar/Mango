#pragma once

#include "Mango/Core/Base.h"
#include "Mango/Core/Math.h"
#include "Formats.h"
#include "Texture.h"

namespace Mango {

	class DepthBuffer {
	public:
		virtual ~DepthBuffer() {}
		virtual void Clear(float value) = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void EnsureSize(uint32_t width, uint32_t height) = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void BindAsTexture(size_t slot) const = 0;

		static DepthBuffer* Create(uint32_t width, uint32_t height);
	};

	void BindRenderTargets(const std::vector<Ref<Texture>>& colorbuffers, const Ref<DepthBuffer>& depthbuffer = nullptr);

}