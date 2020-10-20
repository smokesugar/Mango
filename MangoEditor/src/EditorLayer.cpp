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
		props.Depth = true;
		mFramebuffer1 = Ref<Framebuffer>(Framebuffer::Create(props));
		mFramebuffer2 = Ref<Framebuffer>(Framebuffer::Create(props));
		mFramebuffer3 = Ref<Framebuffer>(Framebuffer::Create(props));

		mFrontBuffer = mFramebuffer1.get();
		mBackBuffer = mFramebuffer2.get();

		mSamplerState = Scope<SamplerState>(SamplerState::Create());
		
		mTAAShader = Ref<Shader>(Shader::Create("assets/shaders/TAA_vs.cso", "assets/shaders/TAA_ps.cso"));

		mTexture = Ref<Texture2D>(Texture2D::Create("assets/textures/Mango.png"));
	}

	inline void EditorLayer::OnUpdate(float dt) {
		Window& window = Application::Get().GetWindow();
		auto buf = window.GetSwapChain().GetFramebuffer();

		auto temp = mFrontBuffer;
		mFrontBuffer = mBackBuffer;
		mBackBuffer = temp;

		Texture::Unbind(0);
		Texture::Unbind(1);
		mFrontBuffer->EnsureSize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);
		mBackBuffer->EnsureSize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);

		mFrontBuffer->Bind();
		mFrontBuffer->Clear(float4(0.1f, 0.1f, 0.1f, 1.0f));

		mScene->SetScreenDimensions((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);
		mScene->OnUpdate(dt);

		// TAA
		mFramebuffer3->EnsureSize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);
		mFramebuffer3->Bind();
		mFramebuffer3->Clear(float4(0.1f, 0.1f, 0.1f, 1.0f));

		mFrontBuffer->BindAsTexture(0);
		mBackBuffer->BindAsTexture(1);
		mSamplerState->Bind(0);
		mTAAShader->Bind();
		Renderer::DrawScreenQuad();
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
						mScene = DataManager::DeserializeScene(path);
						mSceneHierarchyPanel.SetScene(mScene.get());
					}
				}

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
		mViewportFocused = ImGui::IsWindowFocused();
		ImVec2 size = ImGui::GetContentRegionAvail();
		mViewportSize = *(float2*)&size;
		ImGui::Image(Renderer::TAAEnabled() ? mFramebuffer3->GetTextureAttachment() : mFrontBuffer->GetTextureAttachment(), size);
		ImGui::End();
		ImGui::PopStyleVar();

		mSceneHierarchyPanel.OnImGuiRender();

		Dockspace::End();
	}

}