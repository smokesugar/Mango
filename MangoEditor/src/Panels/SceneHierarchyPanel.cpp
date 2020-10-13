#include "SceneHierarchyPanel.h"

#include "Mango/Scene/Components.h"

#include <imgui.h>

namespace Mango {

	SceneHierarchyPanel::SceneHierarchyPanel()
		: mScene(nullptr)
	{
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");
		auto query = mScene->mRegistry.QueryEntities<TagComponent>();
		for (auto& [size, entities, tags] : query)
		{
			for (size_t i = 0; i < size; i++)
			{
				auto id = entities[i];
				auto& tagComponent = tags[i];
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | (mSelectedEntity == id ? ImGuiTreeNodeFlags_Selected : 0);
				bool open = ImGui::TreeNodeEx((void*)&tagComponent, flags, tagComponent.Tag.c_str());
				if (ImGui::IsItemClicked())
					mSelectedEntity = id;
				if (open) {
					ImGui::TreePop();
				}
			}
		}
		ImGui::End();
	}

}