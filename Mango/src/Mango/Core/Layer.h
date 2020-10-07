#pragma once

#include "Mango/Events/Events.h"

namespace Mango {
	
	class Layer {
	public:
		virtual ~Layer() {}

		virtual void OnUpdate() {}
		virtual void OnEvent(Event& e) {}
	};

}