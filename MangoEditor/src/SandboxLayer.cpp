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

		mCamera.GetComponent<CameraComponent>().Camera->SetAspectRatio(mViewportSize.x / mViewportSize.y);

		mScene->OnUpdate(dt);
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

		xmmatrix& mat = mCamera.GetComponent<TransformComponent>().Transform;
		float4 pos;
		XMStoreFloat4(&pos, mat.r[3]);
		ImGui::DragFloat3("Camera Position", ValuePtr(pos), 0.01f);
		mat.r[3] = XMLoadFloat4(&pos);

		ImGui::End();

		mSceneHierarchy.OnImGuiRender();

		Dockspace::End();
	}

}