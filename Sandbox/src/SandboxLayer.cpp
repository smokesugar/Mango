#include "SandboxLayer.h"

#include <imgui.h>

#include "Panels/Dockspace.h"

using namespace DirectX;

struct BruhComponent {
	int i;
};

struct PotatoComponent {
	float bruh;

	PotatoComponent() = default;
	PotatoComponent(float b)
		: bruh(b) {}
};

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

	mTexture = Ref<Texture2D>(Texture2D::Create("assets/textures/Mango.png"));
	mSampler = Ref<SamplerState>(SamplerState::Create());

	// ECS test.
	
	ECS::Registry reg;

	ECS::Entity entities[5];
	for (size_t i = 0; i < std::size(entities); i++) {
		entities[i] = reg.Create();
		reg.Insert<BruhComponent>(entities[i], { (int)i });
	}

	for (size_t i = 0; i < std::size(entities); i++) {
		if (reg.Has<BruhComponent>(entities[i]))
			MG_INFO("{0}", reg.Get<BruhComponent>(entities[i]).i);
	}

	for (size_t i = 0; i < std::size(entities); i++) {
		if (reg.Has<PotatoComponent>(entities[i]))
			MG_INFO("{0}", reg.Get<PotatoComponent>(entities[i]).bruh);
	}

	reg.Destroy(entities[1]);
	reg.Destroy(entities[3]);

	reg.Emplace<PotatoComponent>(entities[0], 10.0f );
	reg.Insert<PotatoComponent>(entities[4], { 4.0f });
	reg.Remove<PotatoComponent>(entities[4]);

	for (size_t i = 0; i < std::size(entities); i++) {
		if(reg.Has<BruhComponent>(entities[i]))
			MG_INFO("{0}", reg.Get<BruhComponent>(entities[i]).i);
	}

	for (size_t i = 0; i < std::size(entities); i++) {
		if (reg.Has<PotatoComponent>(entities[i]))
			MG_INFO("{0}", reg.Get<PotatoComponent>(entities[i]).bruh);
	}
}

inline void SandboxLayer::OnUpdate(float dt) {
	Window& window = Application::Get().GetWindow();
	auto buf = window.GetSwapChain().GetFramebuffer();

	mFramebuffer->EnsureSize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);
	
	mFramebuffer->Bind();
	mFramebuffer->Clear(float4(0.1f, 0.1f, 0.1f, 1.0f));

	mCamera.SetAspectRatio(mViewportSize.x/mViewportSize.y);

	Renderer::BeginScene(mCamera, XMMatrixIdentity());
	mShader->Bind();
	
	mSampler->Bind(0);
	mTexture->Bind(0);

	static float accum = 0;
	accum += dt;
	float s = sinf(accum) * 0.5f + 0.5f;
	xmmatrix transform = XMMatrixScaling(s, s, 1.0);
	Renderer::Submit(mQuad, transform);
	Renderer::EndScene();
}

void SandboxLayer::OnImGuiRender()
{
	Dockspace::Begin();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
	ImGui::Begin("Viewport");
	ImVec2 size = ImGui::GetContentRegionAvail();
	mViewportSize = *(float2*)&size;
	ImGui::Image(mFramebuffer->GetTextureAttachment(), size);
	ImGui::End();
	ImGui::PopStyleVar();

	ImGui::Begin("Properties");
	ImGui::Text("Mango Engine");
	ImGui::End();

	Dockspace::End();
}