#pragma once

namespace Mango {

	namespace RenderCommand {
		void Draw(size_t count, size_t offset);
		void DrawIndexed(size_t count, size_t offset);
		void EnableBlending();
		void DisableBlending();
		void EnableInvertedDepthTesting();
	}

}