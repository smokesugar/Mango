#pragma once

#include "Mango.h"

using namespace Mango;

class SandboxLayer : public Layer {
public:
	SandboxLayer() {
	
	}

	void OnUpdate() {
		MG_CORE_INFO("SandboxLayer::OnUpdate");
	}
};