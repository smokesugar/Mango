#include "EditorLayer.h"

#include "Panels/Dockspace.h"

namespace Mango {

	EditorLayer::EditorLayer()
	{
		mScene = CreateRef<Scene>();

		mSceneHierarchyPanel.SetScene(mScene.get());

		mFramebuffer = Ref<Texture>(Texture::Create(nullptr, 800, 600, Format::RGBA16_FLOAT, Texture_RenderTarget));
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
		ImGui::DragFloat("Environment Strength", &Renderer::EnvironmentStrength(), 0.01f, 0.0f, 100.0f);
		if (ImGui::Button("Select HDRi")) {
			std::string path;
			if (FileDialog::Open(path, L"HDR Texture\0*.hdr\0All\0*.*\0")) {
				Ref<Cubemap> newHdri = Ref<Cubemap>(Cubemap::Create(path, SKYBOX_RESOLUTION));
				Renderer::InitializeCubemap(newHdri);
				Renderer::SetSkybox(newHdri);
			}
		}
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
		ImGui::Image(mFramebuffer->GetNativeTexture(), size);
		mEditorCamera.SetAcceptingInput(ImGui::IsWindowHovered());
		ImGui::End();
		ImGui::PopStyleVar();

		mSceneHierarchyPanel.OnImGuiRender();

		Dockspace::End();
	}

}