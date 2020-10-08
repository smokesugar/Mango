#include "mgpch.h"
#include "Renderer.h"

#include "Mango/Core/Math.h"
#include "Buffer.h"
#include "RenderCommand.h"

using namespace DirectX;

namespace Mango {

	struct RendererData {
		Scope<UniformBuffer> GlobalUniforms;
		Scope<UniformBuffer> IndividualUniforms;
	};

	static RendererData* sData;

	void Renderer::Init()
	{
		sData = new RendererData();
		sData->GlobalUniforms = Scope<UniformBuffer>(UniformBuffer::Create<xmmatrix>());
		sData->IndividualUniforms = Scope<UniformBuffer>(UniformBuffer::Create<xmmatrix>());
	}

	void Renderer::Shutdown()
	{
		delete sData;
	}

	void Renderer::BeginScene(Camera& camera, const xmmatrix& transform)
	{
		xmmatrix viewProjection = XMMatrixInverse(nullptr, transform) * camera.GetProjectionMatrix();
		sData->GlobalUniforms->SetData(viewProjection);
		sData->GlobalUniforms->VSBind(0);
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::Submit(const Ref<VertexArray>& va, const xmmatrix& transform)
	{
		sData->IndividualUniforms->SetData(transform);
		sData->IndividualUniforms->VSBind(1);

		va->Bind();
		if(va->IsIndexed())
			RenderCommand::DrawIndexed(va->GetDrawCount(), 0);
		else
			RenderCommand::Draw(va->GetDrawCount(), 0);
	}

}