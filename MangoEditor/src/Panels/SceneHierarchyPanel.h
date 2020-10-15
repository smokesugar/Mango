#pragma once

#include "Mango/Scene/Scene.h"

namespace Mango {

	class SceneHierarchyPanel {
	public:
		SceneHierarchyPanel();
		
		inline void SetScene(Scene* scene) { mScene = scene; ResetSelectedEntity(); }
		inline void ResetSelectedEntity() { mSelectedEntity = ECS::Null; }

		void OnImGuiRender();
	private:
		Scene* mScene;
		ECS::Entity mSelectedEntity = ECS::Null;
	};

}