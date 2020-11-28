#pragma once

#include "Mango/Core/ECS.h"

namespace Mango {
	namespace ScriptEngine {
		void InitializeScriptComponents(ECS::Registry& reg);
		void ShutdownScriptComponents(ECS::Registry& reg);
		bool UpdateScriptComponents(ECS::Registry& reg, float deltaTime, float elapsedTime);
	}
}