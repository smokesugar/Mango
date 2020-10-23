#include "EditorLayer.h"

#include "Panels/Dockspace.h"

namespace Mango {

	EditorLayer::EditorLayer()
	{
		mScene = CreateRef<Scene>();

		mSceneHierarchyPanel.SetScene(mScene.get());

		FramebufferProperties props;
		props.Width = Application::Get().GetWindow().GetWidth();
		props.Height = Application::Get().GetWindow().GetHeight();
		props.Depth = false;
		mFramebuffer = Ref<Framebuffer>(Framebuffer::Create(props));

		mTexture = Ref<Texture2D>(Texture2D::Create("assets/textures/Mango.png"));
	}

	inline void EditorLayer::OnUpdate(float dt) {
		mFPS = 1.0f / dt;
		mFramebuffer->EnsureSize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);
		mFramebuffer->Clear(RENDERER_CLEAR_COLOR);
		mScene->OnUpdate(dt, mFramebuffer);
	}

	void EditorLayer::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
	}

	void EditorLayer::OnImGuiRender()
	{
		Dockspace::Begin();

		ImGui::Begin("Temp Panel");
		bool& b = Renderer::TAAEnabled();
		ImGui::Text("FPS: %f", mFPS);
		ImGui::Checkbox("TAA", &b);
		ImGui::End();

		// Menu bar
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open")) {
					std::string path;
					if (FileDialog::Open(path)) {
						mScene = Serializer::DeserializeScene(path);
						mSceneHierarchyPanel.SetScene(mScene.get());
					}
				}

				if (ImGui::MenuItem("Save As")) {
					std::string path;
					if (FileDialog::Save(path))
						Serializer::SerializeScene(mScene, path);
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
		mViewportFocused = ImGui::IsWindowFocused();
		ImVec2 size = ImGui::GetContentRegionAvail();
		mViewportSize = *(float2*)&size;
		ImGui::Image(mFramebuffer->GetTextureAttachment(), size);
		ImGui::End();
		ImGui::PopStyleVar();

		mSceneHierarchyPanel.OnImGuiRender();

		Dockspace::End();
	}

}