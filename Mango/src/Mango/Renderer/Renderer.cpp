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
		Ref<Texture2D> WhiteTexture;
		Scope<SamplerState> Sampler;

		Ref<Shader> SpriteShader;
		Ref<Shader> MeshShader;

		bool TAAEnabled = true;

		std::queue<std::tuple<xmmatrix, Ref<Texture2D>, float4>> RenderQueue2D;
		std::queue<std::pair<const Mesh*, xmmatrix>> RenderQueue3D;
	};

	static RendererData* sData;

	void Renderer::Init()
	{
		RenderCommand::EnableBlending();

		sData = new RendererData();

		// Shaders -----------------------------------------------------------------------------------

		sData->SpriteShader = Ref<Shader>(Shader::Create("assets/shaders/Renderer2D_vs.cso", "assets/shaders/Renderer2D_ps.cso"));
		sData->MeshShader = Ref<Shader>(Shader::Create("assets/shaders/Renderer3D_vs.cso", "assets/shaders/Renderer3D_ps.cso"));

		// Textures ----------------------------------------------------------------------------------

		sData->Sampler = Scope<SamplerState>(SamplerState::Create());

		uint32_t color = 0xffffffff;
		sData->WhiteTexture = Ref<Texture2D>(Texture2D::Create(&color, 1, 1));

		// Uniform Buffers ---------------------------------------------------------------------------

		sData->GlobalUniforms = Scope<UniformBuffer>(UniformBuffer::Create<xmmatrix>());
		sData->IndividualUniforms = Scope<UniformBuffer>(UniformBuffer::Create<IndividualData>());

		// Quad --------------------------------------------------------------------------------------

		float vertices[] = {
			  1.0f,  1.0f, 0.0f,   1.0f, 0.0f,
			  1.0f, -1.0f, 0.0f,   1.0f, 1.0f,
			 -1.0f, -1.0f, 0.0f,   0.0f, 1.0f,
			 -1.0f,  1.0f, 0.0f,   0.0f, 0.0f
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

	bool& Renderer::TAAEnabled()
	{
		return sData->TAAEnabled;
	}

	void Renderer::BeginScene(const xmmatrix& projection, const xmmatrix& transform, uint32_t width, uint32_t height)
	{
		static size_t frame = 0;
		frame++;

		if (frame >= 100)
			frame = 0;

		float mult = frame % 2 == 0 ? 1.0f : -1.0f;
		mult *= sData->TAAEnabled ? 1.0f : 0.0f;
		float xJit = 0.5f / (float)width * mult;
		float yJit = 0.5f / (float)height * mult;
		xmmatrix jitterMatrix = XMMatrixTranslation(xJit, yJit, 0.0f);

		xmmatrix viewProjection = XMMatrixInverse(nullptr, transform) * projection * jitterMatrix;
		sData->GlobalUniforms->SetData(viewProjection);
	}

	static void InternalDrawQuad(const xmmatrix& transform, const Ref<Texture2D>& texture, const float4& color) {
		texture->Bind(0);
		xmmatrix halfScale = XMMatrixScaling(0.5f, 0.5f, 1.0f) * transform;
		sData->IndividualUniforms->SetData<IndividualData>({ halfScale, color });
		RenderCommand::DrawIndexed(sData->Quad->GetDrawCount(), 0);
	}

	static void InternalRenderNode(const Node* node, const xmmatrix& parentTransform) {
		xmmatrix transform = node->Transform * parentTransform;
		sData->IndividualUniforms->SetData<IndividualData>({ transform, float4(1.0f, 1.0f, 1.0f, 1.0f) });

		for (auto va : node->Submeshes) {
			va->Bind();
			if (va->IsIndexed())
				RenderCommand::DrawIndexed(va->GetDrawCount(), 0);
			else
				RenderCommand::Draw(va->GetDrawCount(), 0);
		}

		for (auto& node : node->Children) {
			InternalRenderNode(&node, transform);
		}
	}

	void Renderer::EndScene()
	{
		sData->GlobalUniforms->VSBind(0);
		sData->IndividualUniforms->VSBind(1);

		sData->MeshShader->Bind();
		while (!sData->RenderQueue3D.empty()) {
			auto& [mesh, transform] = sData->RenderQueue3D.front();
			InternalRenderNode(&mesh->RootNode, transform);
			sData->RenderQueue3D.pop();
		}

		sData->SpriteShader->Bind();
		sData->Sampler->Bind(0);
		sData->Quad->Bind();
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

	void Renderer::DrawScreenQuad()
	{
		sData->Quad->Bind();
		RenderCommand::DrawIndexed(sData->Quad->GetDrawCount(), 0);
	}

	void Renderer::SubmitMesh(const Mesh& mesh, const xmmatrix& transform)
	{
		sData->RenderQueue3D.push({ &mesh, transform });
	}

}