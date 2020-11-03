#pragma once

#include "Mango/Renderer/RendererAPI.h"

namespace Mango {
	
	class DirectXAPI : public RendererAPI {
	public:
		virtual void Draw(size_t count, size_t offset) override;
		virtual void DrawIndexed(size_t count, size_t offset) override;
		virtual void EnableBlending() override;
		virtual void DisableBlending() override;
		virtual void DefaultRasterizerState() override;
		virtual void ShadowRasterizerState() override;
		virtual void EnableInvertedDepthTesting() override;
	};

}