#pragma once

#include "Mango.h"

using namespace Mango;

class SandboxLayer : public Layer {
public:
	SandboxLayer() {	
	}

	void OnUpdate() {
		Application::Get().GetWindow().GetSwapChain().GetFramebuffer().Clear({0.1f, 0.1f, 0.1f, 1.0f});
	}
};