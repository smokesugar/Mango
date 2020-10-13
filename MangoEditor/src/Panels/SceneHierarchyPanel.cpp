#include "SceneHierarchyPanel.h"

#include <imgui.h>

namespace Mango {

	SceneHierarchyPanel::SceneHierarchyPanel()
		: mScene(nullptr)
	{
	}

	SceneHierarchyPanel::SceneHierarchyPanel(Scene* scene)
		: mScene(scene)
	{
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");
		ImGui::End();
	}

}