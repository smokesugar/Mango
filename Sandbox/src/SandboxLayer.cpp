#include "SandboxLayer.h"

using namespace DirectX;

SandboxLayer::SandboxLayer()
{
	mShader = Ref<Shader>(Shader::Create("assets/shaders/Renderer2D_vs.cso", "assets/shaders/Renderer2D_ps.cso"));

	float vertices[] = {
		0.5f,  0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f
	};
	uint16_t indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	auto vertexBuffer = Ref<VertexBuffer>(VertexBuffer::Create(vertices, 4, 3 * sizeof(float)));
	auto indexBuffer = Ref<IndexBuffer>(IndexBuffer::Create(indices, std::size(indices)));
	mQuad = Ref<VertexArray>(new VertexArray({ vertexBuffer, indexBuffer }));

	mUniformBuffer = Ref<UniformBuffer>(UniformBuffer::Create<float4>());
}

inline void SandboxLayer::OnUpdate(float dt) {
	Window& window = Application::Get().GetWindow();
	auto& framebuffer = window.GetSwapChain().GetFramebuffer();
	framebuffer.Bind();
	framebuffer.Clear(float4(0.1f, 0.1f, 0.1f, 1.0f));

	mCamera.SetAspectRatio((float)window.GetWidth() / (float)window.GetHeight());

	Renderer::BeginScene(mCamera, XMMatrixIdentity());
	mShader->Bind();
	mUniformBuffer->SetData<float4>({ 1.0f, 0.2f, 0.3f, 1.0f });
	mUniformBuffer->PSBind(0);
	Renderer::Submit(mQuad);
	Renderer::EndScene();
}
