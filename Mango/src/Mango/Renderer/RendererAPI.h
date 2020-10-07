#pragma once

namespace Mango {
	
	class RendererAPI {
	public:
		virtual ~RendererAPI() {}

		virtual void SwapBuffers();
	};

}