#pragma once

#include "Mango.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ModelLibraryPanel.h"
#include "EditorCamera.h"

#include <imgui.h>
#include <ImGuizmo.h>

namespace Mango {

	class EditorLayer : public Layer {
	public:
		EditorLayer();
		~EditorLayer();

		void OnUpdate(float dt);
		void OnEvent(Event& e);
		void OnImGuiRender();
	private:
		bool OnKeyDown(KeyDownEvent& e);
		bool OnMouseButtonDown(MouseButtonDownEvent& e);
	private:
		bool mScenePlaying = false;
		Ref<Scene> mScene;

		SceneHierarchyPanel mSceneHierarchyPanel;
		ModelLibraryPanel mModelLibraryPanel;

		Ref<Texture> mFramebuffer;

		float mFPS = 0.0f;

		ImGuizmo::OPERATION mGizmoOperation = ImGuizmo::TRANSLATE;
		float2 mViewportSize = { 800.0f, 600.0f };
		bool mViewportFocused = false;
		bool mViewportHovered = false;
		bool mGizmoHovered = false;
		float2 mViewportMousePosition = {0.0f, 0.0f};

		EditorCamera mEditorCamera;
	};

}