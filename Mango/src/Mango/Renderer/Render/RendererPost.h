#pragma once

#include "Mango/Renderer/Framebuffer.h"

namespace Mango {
	namespace Renderer {
		
		void InitPost();
		void ShutdownPost();

		void TAAPass(const Ref<ColorBuffer>& aliasedFrame, const Ref<ColorBuffer>& velocityBuffer, const Ref<ColorBuffer>& rendertarget);

	}
}