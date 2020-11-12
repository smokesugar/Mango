#pragma once

#include "Mango.h"
#include "Panels/SceneHierarchyPanel.h"
#include "EditorCamera.h"

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
		bool OnKeyDown(KeyDownEvent& e);
		bool OnMouseButtonDown(MouseButtonDownEvent& e);
		void RenderNode(const Node& node, const xmmatrix& parentTransform, float3 color);
	private:
		bool mScenePlaying = false;
		Ref<Scene> mScene;

		SceneHierarchyPanel mSceneHierarchyPanel;

		Ref<Texture> mFramebuffer;

		float mFPS = 0.0f;

		ImGuizmo::OPERATION mGizmoOperation = ImGuizmo::TRANSLATE;
		float2 mViewportSize = { 800.0f, 600.0f };
		bool mViewportFocused = false;
		bool mViewportHovered = false;
		bool mGizmoHovered = false;
		float2 mViewportMousePosition = {0.0f, 0.0f};

		EditorCamera mEditorCamera;

		Ref<Texture> mMousePickerTexture;
		Ref<DepthBuffer> mMousePickerDepthBuffer;
		Ref<Texture> mMousePickerTextureStaging;
		Ref<Shader> mMousePickerShader;
		Ref<UniformBuffer> mMousePickerUniforms;
	};

}