#pragma once

#include "Mango.h"

using namespace Mango;

class SandboxLayer : public Layer {
public:
	SandboxLayer()
	{	
		mShader = Ref<Shader>(Shader::Create("assets/shaders/Renderer2D_vs.cso", "assets/shaders/UVColors_ps.cso"));

		float vertices[] = {
			 0.5f,  0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			-0.5f, -0.5f, 0.0f,
			-0.5f,  0.5f, 0.0f
		};

		mVertexBuffer = Ref<VertexBuffer>(VertexBuffer::Create(vertices, 4, 3 * sizeof(float)));

		uint16_t indices[] = {
			0, 1, 3,
			1, 2, 3
		};
		
		mIndexBuffer = Ref<IndexBuffer>(IndexBuffer::Create(indices, std::size(indices)));

		mUniformBuffer = Ref<UniformBuffer>(UniformBuffer::Create<ColorBuffer>());
	}

	void OnUpdate() {
		auto& mainFramebuffer = Application::Get().GetWindow().GetSwapChain().GetFramebuffer();
		mainFramebuffer.Bind();
		mainFramebuffer.Clear(float4(0.1f, 0.1f, 0.1f, 1.0f));
		mShader->Bind();
		mVertexBuffer->Bind();
		mIndexBuffer->Bind();
		mUniformBuffer->SetData<ColorBuffer>({ {1.0, 1.0, 1.0, 1.0} });
		mUniformBuffer->PSBind(0);
		RenderCommand::DrawIndexed(6, 0);
	}
private:
	Ref<Shader> mShader;
	Ref<VertexBuffer> mVertexBuffer;
	Ref<IndexBuffer> mIndexBuffer;

	struct ColorBuffer {
		float4 color;
	};
	Ref<UniformBuffer> mUniformBuffer;
};