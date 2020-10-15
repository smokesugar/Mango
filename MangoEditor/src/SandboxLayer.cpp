#include "SandboxLayer.h"

#include <imgui.h>

#include "Panels/Dockspace.h"

namespace Mango {

	SandboxLayer::SandboxLayer()
	{
		mScene = CreateRef<Scene>();
		mSceneHierarchy.SetScene(mScene.get());

		FramebufferProperties props;
		props.Width = Application::Get().GetWindow().GetWidth();
		props.Height = Application::Get().GetWindow().GetHeight();
		props.Depth = true;
		mFramebuffer = Ref<Framebuffer>(Framebuffer::Create(props));

		mTexture = Ref<Texture2D>(Texture2D::Create("assets/textures/Mango.png"));

		Entity entity = mScene->Create("Square");
		entity.AddComponent<SpriteRendererComponent>(float4(0.2f, 0.3f, 1.0f, 1.0f));

		mCamera = mScene->Create("Main Camera");
		mCamera.AddComponent<CameraComponent>(Ref<Camera>(new OrthographicCamera())).Primary = true;
	}

	inline void SandboxLayer::OnUpdate(float dt) {
		Window& window = Application::Get().GetWindow();
		auto buf = window.GetSwapChain().GetFramebuffer();

		mFramebuffer->EnsureSize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);

		mFramebuffer->Bind();
		mFramebuffer->Clear(float4(0.1f, 0.1f, 0.1f, 1.0f));

		mScene->SetAspectRatio(mViewportSize.x / mViewportSize.y);
		mScene->OnUpdate(dt);
	}

	void SandboxLayer::OnImGuiRender()
	{
		Dockspace::Begin();

		// Menu bar
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save As")) {
					std::string path;
					if (FileDialog::Save(path))
						DataManager::SerializeScene(mScene, path);
				}

				if (ImGui::MenuItem("Exit"))
					Mango::Application::Get().Close();

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		// Viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		ImGui::Begin("Viewport");
		ImVec2 size = ImGui::GetContentRegionAvail();
		mViewportSize = *(float2*)&size;
		ImGui::Image(mFramebuffer->GetTextureAttachment(), size);
		ImGui::End();
		ImGui::PopStyleVar();

		// Scene Hierarchy
		mSceneHierarchy.OnImGuiRender();

		Dockspace::End();
	}

}