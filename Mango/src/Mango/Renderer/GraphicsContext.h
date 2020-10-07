#pragma once

namespace Mango {
	
	class GraphicsContext {
	public:
		virtual ~GraphicsContext() {}

		static GraphicsContext* Create();
	};

}