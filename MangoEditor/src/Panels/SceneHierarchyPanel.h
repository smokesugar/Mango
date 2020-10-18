#pragma once

#include "Mango/Scene/Scene.h"

namespace Mango {

	class SceneHierarchyPanel {
	public:
		SceneHierarchyPanel();
		
		inline void SetScene(Scene* scene) { mScene = scene; ResetSelectedEntity(); }
		inline void ResetSelectedEntity() { mSelectedEntity = ECS::Null; }

		inline ECS::Entity GetSelectedEntity() const { return mSelectedEntity; }

		void OnImGuiRender();
	private:
		template<typename T>
		void SetDeleteTypeOnRightClick() {
			if (ImGui::IsItemClicked(1)) {
				mCompDeleteFunction = [&]() {
					Entity ent(mSelectedEntity, mScene);
					ent.RemoveComponent<T>();
				};
				ImGui::OpenPopup("remove_component");
			}
		}
	private:
		Scene* mScene;
		ECS::Entity mSelectedEntity = ECS::Null;
		std::function<void()> mCompDeleteFunction;
	};

}