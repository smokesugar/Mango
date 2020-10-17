#include "mgpch.h"
#include "Renderer.h"

#include "Mango/Core/Math.h"
#include "Buffer.h"
#include "RenderCommand.h"
#include "Shader.h"

namespace Mango {

	struct IndividualData {
		xmmatrix ViewProjection;
		float4 color;
	};

	struct RendererData {
		Scope<UniformBuffer> GlobalUniforms;
		Scope<UniformBuffer> IndividualUniforms;
		Ref<VertexArray> Quad;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;
		Scope<SamplerState> Sampler;

		std::queue<std::tuple<xmmatrix, Ref<Texture2D>, float4>> RenderQueue2D;
	};

	static RendererData* sData;

	void Renderer::Init()
	{
		RenderCommand::EnableBlending();

		sData = new RendererData();

		// Shaders -----------------------------------------------------------------------------------

		sData->TextureShader = Ref<Shader>(Shader::Create("assets/shaders/Renderer2D_vs.cso", "assets/shaders/Renderer2D_ps.cso"));

		// Textures ----------------------------------------------------------------------------------

		sData->Sampler = Scope<SamplerState>(SamplerState::Create());

		uint32_t color = 0xffffffff;
		sData->WhiteTexture = Ref<Texture2D>(Texture2D::Create(&color, 1, 1));

		// Uniform Buffers ---------------------------------------------------------------------------

		sData->GlobalUniforms = Scope<UniformBuffer>(UniformBuffer::Create<xmmatrix>());
		sData->IndividualUniforms = Scope<UniformBuffer>(UniformBuffer::Create<IndividualData>());

		// Quad --------------------------------------------------------------------------------------

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
		sData->Quad = CreateRef<VertexArray>(vertexBuffer, indexBuffer);
	}

	void Renderer::Shutdown()
	{
		delete sData;
	}

	void Renderer::BeginScene(Camera& camera, const xmmatrix& transform)
	{
		xmmatrix viewProjection = XMMatrixInverse(nullptr, transform) * camera.GetProjectionMatrix();
		sData->GlobalUniforms->SetData(viewProjection);
	}

	static void InternalDrawQuad(const xmmatrix& transform, const Ref<Texture2D>& texture, const float4& color) {
		texture->Bind(0);
		sData->IndividualUniforms->SetData<IndividualData>({ transform, color });
		RenderCommand::DrawIndexed(sData->Quad->GetDrawCount(), 0);
	}

	void Renderer::EndScene()
	{
		sData->GlobalUniforms->VSBind(0);
		sData->IndividualUniforms->VSBind(1);
		sData->TextureShader->Bind();
		sData->Quad->Bind();
		sData->Sampler->Bind(0);

		while (!sData->RenderQueue2D.empty()) {
			auto& tuple = sData->RenderQueue2D.front();
			InternalDrawQuad(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple));
			sData->RenderQueue2D.pop();
		}
	}

	void Renderer::DrawQuad(const float3& pos, const float2& size, const float4& color)
	{
		xmmatrix transform = XMMatrixScaling(size.x, size.y, 1.0f) * XMMatrixTranslation(pos.x, pos.y, pos.z);
		sData->RenderQueue2D.push({transform, sData->WhiteTexture, color});
	}

	void Renderer::DrawQuad(const float3& pos, const float2& size, const Ref<Texture2D>& texture)
	{
		xmmatrix transform = XMMatrixScaling(size.x, size.y, 1.0f) * XMMatrixTranslation(pos.x, pos.y, pos.z);
		sData->RenderQueue2D.push({ transform, texture, float4(1.0f, 1.0f, 1.0f, 1.0f) });
	}

	void Renderer::DrawQuad(const xmmatrix& transform, const float4& color)
	{
		sData->RenderQueue2D.push({ transform, sData->WhiteTexture, color });
	}

	void Renderer::DrawQuad(const xmmatrix& transform, const Ref<Texture2D>& texture)
	{
		sData->RenderQueue2D.push({ transform, texture, float4(1.0f, 1.0f, 1.0f, 1.0f) });
	}

}