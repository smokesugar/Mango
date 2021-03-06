#pragma once

#include <cinttypes>
#include "Mango/Core/Math.h"

namespace Mango {
	
	class SwapChain {
	public:
		virtual ~SwapChain() {}
		virtual void Present() = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void BindAsRenderTarget() = 0;
		static SwapChain* Create();
	};

}