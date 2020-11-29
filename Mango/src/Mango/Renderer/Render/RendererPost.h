#pragma once

#include "Mango/Renderer/RenderTarget.h"

namespace Mango {
	namespace Renderer {
		
		void InitPost();
		void ShutdownPost();

		void TAAPass(const Ref<Texture>& aliasedFrame, const Ref<Texture>& velocityBuffer, const Ref<Texture>& rendertarget);
		void SSAOPass(const Ref<DepthBuffer>& depthBuffer, const Ref<Texture>& normalBuffer, const Ref<Texture>& rendertarget);
	}
}