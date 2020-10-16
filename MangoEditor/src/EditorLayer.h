#pragma once

#include "Mango.h"
#include "Panels/SceneHierarchyPanel.h"

namespace Mango {

	class EditorLayer : public Layer {
	public:
		EditorLayer();

		void OnUpdate(float dt);
		void OnImGuiRender();
	private:
		void DrawGizmo();
	private:
		Ref<Scene> mScene;

		SceneHierarchyPanel mSceneHierarchy;

		Ref<Framebuffer> mFramebuffer;
		Ref<Texture2D> mTexture;

		float2 mViewportSize = { 800.0f, 600.0f };
	};

}