#pragma once

#include "Mango.h"
#include "Panels/SceneHierarchyPanel.h"

#include <imgui.h>
#include <ImGuizmo.h>

namespace Mango {

	class EditorLayer : public Layer {
	public:
		EditorLayer();

		void OnUpdate(float dt);
		void OnEvent(Event& e);
		void OnImGuiRender();
	private:
		Ref<Scene> mScene;

		SceneHierarchyPanel mSceneHierarchyPanel;

		Ref<Framebuffer> mFramebuffer;
		Ref<Texture2D> mTexture;

		float2 mViewportSize = { 800.0f, 600.0f };
		bool mViewportFocused = false;
	};

}