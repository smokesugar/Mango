#pragma once

#include "Mango/Scene/Scene.h"

namespace Mango {

	class SceneHierarchyPanel {
	public:
		SceneHierarchyPanel();
		SceneHierarchyPanel(Scene* scene);

		void OnImGuiRender();
	private:
		Scene* mScene;
	};

}