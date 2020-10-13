#pragma once

#include "Mango/Scene/Scene.h"

namespace Mango {

	class SceneHierarchyPanel {
	public:
		SceneHierarchyPanel();
		
		inline void SetScene(Scene* scene) { mScene = scene; }

		void OnImGuiRender();
	private:
		Scene* mScene;
		ECS::Entity mSelectedEntity = ECS::Null;
	};

}