#pragma once

#include "Mango.h"
#include "Panels/SceneHierarchyPanel.h"

namespace Mango {

	class SandboxLayer : public Layer {
	public:
		SandboxLayer();

		void OnUpdate(float dt);
		void OnImGuiRender();
	private:
		Ref<Scene> mScene;

		SceneHierarchyPanel mSceneHierarchy;

		Ref<Framebuffer> mFramebuffer;
		Ref<Texture2D> mTexture;

		float2 mViewportSize = { 800.0f, 600.0f };
	};

}