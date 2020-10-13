#include "SandboxLayer.h"

#include <imgui.h>

#include "Panels/Dockspace.h"

namespace Mango {

	SandboxLayer::SandboxLayer()
	{
		mScene = CreateRef<Scene>();
		mSceneHierarchy = CreateScope<SceneHierarchyPanel>(mScene.get());

		FramebufferProperties props;
		props.Width = Application::Get().GetWindow().GetWidth();
		props.Height = Application::Get().GetWindow().GetHeight();
		props.Depth = true;
		mFramebuffer = Ref<Framebuffer>(Framebuffer::Create(props));

		mTexture = Ref<Texture2D>(Texture2D::Create("assets/textures/Mango.png"));

		Entity entity = mScene->Create();

		entity.AddComponent<SpriteRendererComponent>(float4(0.2f, 0.3f, 1.0f, 1.0f));
	}

	inline void SandboxLayer::OnUpdate(float dt) {
		Window& window = Application::Get().GetWindow();
		auto buf = window.GetSwapChain().GetFramebuffer();

		mFramebuffer->EnsureSize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);

		mFramebuffer->Bind();
		mFramebuffer->Clear(float4(0.1f, 0.1f, 0.1f, 1.0f));

		mCamera.SetAspectRatio(mViewportSize.x / mViewportSize.y);

		Renderer::BeginScene(mCamera, XMMatrixIdentity());

		static float accum = 0;
		accum += dt;
		float s = sinf(accum) * 0.5f + 0.5f;

		mScene->OnUpdate(dt);

		Renderer::EndScene();
	}

	void SandboxLayer::OnImGuiRender()
	{
		Dockspace::Begin();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		ImGui::Begin("Viewport");
		ImVec2 size = ImGui::GetContentRegionAvail();
		mViewportSize = *(float2*)&size;
		ImGui::Image(mFramebuffer->GetTextureAttachment(), size);
		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::Begin("Properties");
		ImGui::Text("Mango Engine");
		ImGui::ColorEdit4("Square Color", ValuePtr(mSquareColor));
		ImGui::End();

		mSceneHierarchy->OnImGuiRender();

		Dockspace::End();
	}

}