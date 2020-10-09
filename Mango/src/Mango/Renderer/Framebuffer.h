#pragma once

#include "Mango/Core/Base.h"
#include "Mango/Core/Math.h"

namespace Mango {

	struct FramebufferProperties {
		uint32_t Width;
		uint32_t Height;
	};
	
	class Framebuffer {
	public:
		virtual ~Framebuffer() {}
		virtual void Bind() = 0;
		virtual void Clear(const float4& color) = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void EnsureSize(uint32_t width, uint32_t height) = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		
		static void Blit(const Ref<Framebuffer>& dst, const Ref<Framebuffer>& src);

		static Framebuffer* Create(const FramebufferProperties& props);
	};

}