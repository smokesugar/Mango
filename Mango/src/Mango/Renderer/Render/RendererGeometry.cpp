#include "mgpch.h"
#include "RendererGeometry.h"

#include "Mango/Renderer/RenderCommand.h"
#include "Mango/Renderer/Shader.h"
#include "Mango/Renderer/Buffer.h"

#include "RendererBase.h"

namespace Mango {

	struct TransformData {
		xmmatrix Transform;
		xmmatrix PrevMVP;
		xmmatrix MVP;
	};

	struct SurfaceData {
		float3 AlbedoColor;
		int UseNormalMap;
		float RoughnessValue;
		float Metalness;
		float2 _padding;
	};

	struct RenderDataGeo {
		Ref<Shader> GeometryShader;
		Ref<UniformBuffer> SurfaceUniforms;
		Ref<UniformBuffer> TransformUniforms;
	};

	static RenderDataGeo* sData;

	void Renderer::InitGeometry()
	{
		sData = new RenderDataGeo();

		sData->GeometryShader = Ref<Shader>(Shader::Create("assets/shaders/GeometryPass_vs.cso", "assets/shaders/GeometryPass_ps.cso"));

		sData->SurfaceUniforms = Ref<UniformBuffer>(UniformBuffer::Create<SurfaceData>());
		sData->TransformUniforms = Ref<UniformBuffer>(UniformBuffer::Create<TransformData>());
	}

	void Renderer::ShutdownGeometry()
	{
		delete sData;
	}

	void Renderer::RenderToGBuffer(std::unordered_map<Ref<Material>, std::vector<std::tuple<Ref<VertexArray>, xmmatrix, xmmatrix>>>& queue, const std::vector<Ref<Texture>>& rendertargets, const Ref<DepthBuffer>& depthBuffer)
	{
		RenderCommand::DisableBlending();
		BindRenderTargets(rendertargets, depthBuffer);
		for (auto& buffer : rendertargets)
			buffer->Clear(float4(0.0f, 0.0f, 0.0f, 1.0f));
		depthBuffer->Clear(0.0f);

		LinearSampler().Bind(0);
		sData->GeometryShader->Bind();
		sData->SurfaceUniforms->PSBind(0);
		sData->TransformUniforms->VSBind(0);

		for (auto& [material, submeshes] : queue) {
			sData->SurfaceUniforms->SetData<SurfaceData>({ material->AlbedoColor, material->NormalTexture ? true : false, material->RoughnessValue, material->Metalness });
			material->AlbedoTexture->Bind(0);
			if (material->NormalTexture)
				material->NormalTexture->Bind(1);
			material->RoughnessTexture->Bind(2);

			xmmatrix prevVP = GetPrevViewMatrix() * GetPrevProjectionMatrix();
			xmmatrix VP = GetViewMatrix() * GetProjectionMatrix() * GetJitterMatrix();

			for (auto& [va, prevT, transform] : submeshes) {
				xmmatrix prevMVP = prevT * prevVP;
				xmmatrix MVP = transform * VP;
				sData->TransformUniforms->SetData<TransformData>({transform, prevMVP, MVP});

				va->Bind();
				if (va->IsIndexed())
					RenderCommand::DrawIndexed(va->GetDrawCount(), 0);
				else
					RenderCommand::Draw(va->GetDrawCount(), 0);
			}
		}
	}

}
