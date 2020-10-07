#pragma once

#include "RendererAPI.h"

namespace Mango {

	class RenderCommand {
	public:
		static void Draw(size_t count, size_t offset);
		static void DrawIndexed(size_t count, size_t offset);
	private:
		static RendererAPI* sRendererAPI;
	};

}