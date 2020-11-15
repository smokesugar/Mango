#pragma once

#include "Mango.h"

namespace Mango {
	
	class ModelLibraryPanel {
	public:
		ModelLibraryPanel() = default;
		void SetScene(Scene* scene) { mScene = scene; }

		void OnImGuiRender();
	private:
		Scene* mScene = nullptr;
	};

}