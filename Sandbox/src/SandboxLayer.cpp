#include "SandboxLayer.h"

#include <imgui.h>

using namespace DirectX;

SandboxLayer::SandboxLayer()
{
	mShader = Ref<Shader>(Shader::Create("assets/shaders/Renderer2D_vs.cso", "assets/shaders/Renderer2D_ps.cso"));

	float vertices[] = {
		  0.5f,  0.5f, 0.0f,   1.0f, 0.0f,
		  0.5f, -0.5f, 0.0f,   1.0f, 1.0f,
		 -0.5f, -0.5f, 0.0f,   0.0f, 1.0f,
		 -0.5f,  0.5f, 0.0f,   0.0f, 0.0f
	};
	uint16_t indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	auto vertexBuffer = Ref<VertexBuffer>(VertexBuffer::Create(vertices, 4, 5 * sizeof(float)));
	auto indexBuffer = Ref<IndexBuffer>(IndexBuffer::Create(indices, std::size(indices)));
	mQuad = Ref<VertexArray>(new VertexArray({ vertexBuffer, indexBuffer }));

	FramebufferProperties props;
	props.Width = Application::Get().GetWindow().GetWidth();
	props.Height = Application::Get().GetWindow().GetHeight();
	mFramebuffer = Ref<Framebuffer>(Framebuffer::Create(props));

	mTexture = Ref<Texture2D>(Texture2D::Create("assets/textures/MangoTexture.png"));
	mSampler = Ref<SamplerState>(SamplerState::Create());
}

inline void SandboxLayer::OnUpdate(float dt) {
	Window& window = Application::Get().GetWindow();
	auto buf = window.GetSwapChain().GetFramebuffer();

	mFramebuffer->EnsureSize(window.GetWidth(), window.GetHeight());
	
	mFramebuffer->Bind();
	mFramebuffer->Clear(float4(0.1f, 0.1f, 0.1f, 1.0f));

	mCamera.SetAspectRatio((float)window.GetWidth() / (float)window.GetHeight());

	Renderer::BeginScene(mCamera, XMMatrixIdentity());
	mShader->Bind();
	
	mSampler->Bind(0);
	mTexture->Bind(0);

	Renderer::Submit(mQuad);
	Renderer::EndScene();

	Framebuffer::Blit(buf, mFramebuffer);
}

void SandboxLayer::OnImGuiRender()
{
	ImGui::ShowDemoWindow();
}