#include "mgpch.h"
#include "RendererSprite.h"

#include "Mango/Renderer/Shader.h"
#include "Mango/Renderer/RenderCommand.h"
#include "Mango/Renderer/Buffer.h"

#include "RendererBase.h"

namespace Mango {

	struct SpriteData {
		xmmatrix PreviousMVP;
		xmmatrix MVP;
		float4 Color;
	};

	struct RenderDataSprite {
		Ref<UniformBuffer> SpriteUniforms;
		Ref<VertexArray> Quad;
		Ref<Shader> SpriteShader;
	};

	static RenderDataSprite* sData;

	void Renderer::InitSprite()
	{
		sData = new RenderDataSprite();

		sData->SpriteShader = Ref<Shader>(Shader::Create("assets/shaders/Sprite_vs.cso", "assets/shaders/Sprite_ps.cso"));

		sData->SpriteUniforms = Ref<UniformBuffer>(UniformBuffer::Create<SpriteData>());

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

	void Renderer::ShutdownSprite()
	{
		delete sData;
	}

	const Ref<VertexArray>& Renderer::GetSpriteQuadVertexArray()
	{
		return sData->Quad;
	}

	static void DrawQuad(const xmmatrix& previousTransform, const xmmatrix& transform, const Ref<Texture>& texture, const float4& color) {
		texture->Bind(0);
		xmmatrix prevMVP = previousTransform * Renderer::GetPrevViewMatrix() * Renderer::GetPrevProjectionMatrix() * Renderer::GetJitterMatrix();
		xmmatrix MVP = transform * Renderer::GetViewMatrix() * Renderer::GetProjectionMatrix() * Renderer::GetJitterMatrix();
		sData->SpriteUniforms->SetData<SpriteData>({prevMVP, MVP, color});
		RenderCommand::DrawIndexed(sData->Quad->GetDrawCount(), 0);
	}

	void Renderer::FlushSpriteQueue(std::queue<std::tuple<xmmatrix, xmmatrix, Ref<Texture>, float4>>& queue, const std::vector<Ref<Texture>>& rendertargets, const Ref<DepthBuffer>& depthbuffer)
	{
		BindRenderTargets(rendertargets, depthbuffer);
		RenderCommand::EnableBlending();
		Renderer::LinearSampler().Bind(0);
		sData->SpriteShader->Bind();
		sData->Quad->Bind();
		sData->SpriteUniforms->VSBind(0);
		while (!queue.empty()) {
			auto& [prevTransform, transform, texture, color] = queue.front();
			DrawQuad(prevTransform, transform, texture, color);
			queue.pop();
		}
		RenderCommand::DisableBlending();
	}

}
