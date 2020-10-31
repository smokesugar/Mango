#include "EditorLayer.h"

#include "Panels/Dockspace.h"

namespace Mango {

	EditorLayer::EditorLayer()
	{
		mScene = CreateRef<Scene>();

		mSceneHierarchyPanel.SetScene(mScene.get());

		ColorBufferProperties props;
		props.Width = 800;
		props.Height = 600;
		props.Format = Format::RGBA16_FLOAT;
		mFramebuffer = Ref<ColorBuffer>(ColorBuffer::Create(props));

		mTexture = Ref<Texture2D>(Texture2D::Create("assets/textures/Mango.png"));
	}

	inline void EditorLayer::OnUpdate(float dt) {
		mFPS = 1.0f / dt;
		mFramebuffer->EnsureSize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);
		mFramebuffer->Clear(RENDERER_CLEAR_COLOR);
		if(mScenePlaying)
			mScene->OnUpdate(dt, mFramebuffer);
		else
			mScene->OnUpdate(dt, mFramebuffer, mEditorCamera.GetProjectionMatrix(mViewportSize.x/mViewportSize.y), mEditorCamera.GetTransform());
	}

	void EditorLayer::OnEvent(Event& e)
	{
		mEditorCamera.OnEvent(e);
	}

	void EditorLayer::OnImGuiRender()
	{
		Dockspace::Begin();

		ImGui::Begin("Temp Panel");
		bool& b = Renderer::TAAEnabled();
		ImGui::Text("FPS: %f", mFPS);
		ImGui::Checkbox("TAA", &b);
		ImGui::Checkbox("Scene Playing", &mScenePlaying);
		ImGui::End();

		// Menu bar
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open")) {
					std::string path;
					if (FileDialog::Open(path, L"Mango Scene\0*.json;*.mango\0All\0*.*\0")) {
						mScene = Serializer::DeserializeScene(path);
						mSceneHierarchyPanel.SetScene(mScene.get());
					}
				}

				if (ImGui::MenuItem("Save As")) {
					std::string path;
					if (FileDialog::Save(path, L"Mango Scene\0*.json;*.mango\0All\0*.*\0"))
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
		mEditorCamera.SetAcceptingInput(ImGui::IsWindowHovered());
		ImGui::End();
		ImGui::PopStyleVar();

		mSceneHierarchyPanel.OnImGuiRender();

		Dockspace::End();
	}

}