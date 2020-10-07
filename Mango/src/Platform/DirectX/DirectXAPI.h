#pragma once

#include "Mango/Renderer/RendererAPI.h"

namespace Mango {
	
	class DirectXAPI : public RendererAPI {
	public:
		virtual void Draw(size_t count, size_t offset);
		virtual void DrawIndexed(size_t count, size_t offset);
	};

}