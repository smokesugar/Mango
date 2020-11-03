#pragma once

namespace Mango {
	
	class RendererAPI {
	public:
		virtual ~RendererAPI() {}
		virtual void Draw(size_t count, size_t offset) = 0;
		virtual void DrawIndexed(size_t count, size_t offset) = 0;

		virtual void EnableBlending() = 0;
		virtual void DisableBlending() = 0;
		virtual void DefaultRasterizerState() = 0;
		virtual void ShadowRasterizerState() = 0;

		virtual void EnableInvertedDepthTesting() = 0;

		static RendererAPI* Create();
	};

}