#include "SandboxLayer.h"

#include <imgui.h>

SandboxLayer::SandboxLayer()
{
	mScene = CreateRef<Scene>();

	FramebufferProperties props;
	props.Width = Application::Get().GetWindow().GetWidth();
	props.Height = Application::Get().GetWindow().GetHeight();
	props.Depth = true;
	mFramebuffer = Ref<Framebuffer>(Framebuffer::Create(props));

	mTexture = Ref<Texture2D>(Texture2D::Create("assets/textures/Mango.png"));
}

inline void SandboxLayer::OnUpdate(float dt) {
	Window& window = Application::Get().GetWindow();
	auto buf = window.GetSwapChain().GetFramebuffer();

	mFramebuffer->EnsureSize(window.GetWidth(), window.GetHeight());
	
	mFramebuffer->Bind();
	mFramebuffer->Clear(float4(0.1f, 0.1f, 0.1f, 1.0f));

	mCamera.SetAspectRatio((float)window.GetWidth()/(float)window.GetHeight());

	Renderer::BeginScene(mCamera, XMMatrixIdentity());

	static float accum = 0;
	accum += dt;
	float s = sinf(accum) * 0.5f + 0.5f;

	Renderer::DrawQuad(float3(0.0f, 0.0f, 0.0f), float2(1.0f, 1.0f), mTexture);
	Renderer::DrawQuad(float3(0.0f, 0.0f, -0.5f), float2(1.0f, 1.0f), float4(1.0f, 0.2f, 0.3f, 1.0f));

	Renderer::EndScene();

	mScene->OnUpdate(dt);

	Framebuffer::Blit(buf, mFramebuffer);
}