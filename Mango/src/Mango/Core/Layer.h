#pragma once

#include "Mango/Events/Events.h"

namespace Mango {
	
	class Layer {
	public:
		virtual ~Layer() {}

		virtual void OnUpdate(float dt) {}
		virtual void OnEvent(Event& e) {}
		virtual void OnImGuiRender() {}
	};

}