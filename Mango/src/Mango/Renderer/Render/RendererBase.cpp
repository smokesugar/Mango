#include "mgpch.h"
#include "RendererBase.h"

#include "Mango/Renderer/VertexArray.h"
#include "Mango/Renderer/RenderCommand.h"

namespace Mango {

	struct RenderDataBase {
		xmmatrix ViewMatrix;
		xmmatrix ProjectionMatrix;
		xmmatrix JitterMatrix;
		xmmatrix PrevViewMatrix;
		xmmatrix PrevProjectionMatrix;

		Ref<VertexArray> Quad;
		Scope<SamplerState> LinearSampler;
		Scope<SamplerState> LinearSamplerClamp;
		Scope<SamplerState> PointSampler;
	};

	static RenderDataBase* sData = nullptr;
	
	void Renderer::InitBase() {
		sData = new RenderDataBase();

		sData->LinearSampler = Scope<SamplerState>(SamplerState::Create(SamplerState::Filter::Linear));
		sData->LinearSamplerClamp = Scope<SamplerState>(SamplerState::Create(SamplerState::Filter::Linear, SamplerState::Address::Clamp));
		sData->PointSampler =  Scope<SamplerState>(SamplerState::Create(SamplerState::Filter::Point));

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

	void Renderer::ShutdownBase()
	{
		delete sData;
	}

	void Renderer::DrawScreenQuad()
	{
		sData->Quad->Bind();
		RenderCommand::DrawIndexed(sData->Quad->GetDrawCount(), 0);
	}

	SamplerState& Renderer::LinearSampler()
	{
		return *sData->LinearSampler;
	}

	SamplerState& Renderer::LinearSamplerClamp()
	{
		return *sData->LinearSamplerClamp;
	}

	SamplerState& Renderer::PointSampler()
	{
		return *sData->PointSampler;
	}

	void Renderer::StoreViewMatrix(const xmmatrix& view)
	{
		sData->PrevViewMatrix = sData->ViewMatrix;
		sData->ViewMatrix = view;
	}

	void Renderer::StoreProjectionMatrix(const xmmatrix& projection)
	{
		sData->PrevProjectionMatrix = sData->ProjectionMatrix;
		sData->ProjectionMatrix = projection;
	}

	void Renderer::StoreJitterMatrix(const xmmatrix& jitter)
	{
		sData->JitterMatrix = jitter;
	}

	const xmmatrix& Renderer::GetViewMatrix()
	{
		return sData->ViewMatrix;
	}

	const xmmatrix& Renderer::GetProjectionMatrix()
	{
		return sData->ProjectionMatrix;
	}

	const xmmatrix& Renderer::GetPrevViewMatrix()
	{
		return sData->PrevViewMatrix;
	}

	const xmmatrix& Renderer::GetPrevProjectionMatrix()
	{
		return sData->PrevProjectionMatrix;
	}

	const xmmatrix& Renderer::GetJitterMatrix()
	{
		return sData->JitterMatrix;
	}

}