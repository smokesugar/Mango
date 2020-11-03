#pragma once

#include "Mango/Core/Base.h"
#include "Mango/Core/Math.h"
#include "Formats.h"

namespace Mango {

	struct ColorBufferProperties {
		uint32_t Width;
		uint32_t Height;
		Mango::Format Format;
	};
	
	class ColorBuffer {
	public:
		virtual ~ColorBuffer() {}
		virtual void Clear(const float4& color) = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void EnsureSize(uint32_t width, uint32_t height) = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void* GetTextureAttachment() const = 0;
		virtual void BindAsTexture(size_t slot) const = 0;
		
		static void Blit(const Ref<ColorBuffer>& dst, const Ref<ColorBuffer>& src);

		static ColorBuffer* Create(const ColorBufferProperties& props);
	};

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

	void BindRenderTargets(const std::vector<Ref<ColorBuffer>>& colorbuffers, const Ref<DepthBuffer>& depthbuffer = nullptr);
}