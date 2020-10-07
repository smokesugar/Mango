#pragma once

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

		static Framebuffer* Create(const FramebufferProperties& props);
	};

}