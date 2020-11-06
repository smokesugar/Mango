#include "mgpch.h"
#include "RendererLighting.h"

#include "Mango/Renderer/Buffer.h"
#include "Mango/Renderer/CascadedShadowmap.h"
#include "Mango/Renderer/Shader.h"
#include "Mango/Renderer/RenderCommand.h"

#include "RendererBase.h"

#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 16
#define NUM_SHADOW_CASCADES 4

namespace Mango {

	struct Light {
		float3 Vector;
		float padding0;
		float3 Color;
		float padding1;
	};

	struct LightingData {
		xmmatrix InvView;
		Light PointLights[MAX_POINT_LIGHTS];
		Light DirectionalLights[MAX_DIRECTIONAL_LIGHTS];
		xmmatrix DirectionalMatrices[MAX_DIRECTIONAL_LIGHTS * NUM_SHADOW_CASCADES];
		int NumDirectionalLights = 0;
		int NumPointLights = 0;
		float2 padding;
		float4 PerspectiveValues;
		float CascadeEnds[NUM_SHADOW_CASCADES];
	};

	struct RenderDataLighting {
		Ref<SamplerState> ShadowSampler;
		Ref<CascadedShadowmap> DirectionalShadowmaps[MAX_DIRECTIONAL_LIGHTS];

		Ref<Shader> LightingShader;
		Ref<Shader> DirectionalShadowmapShader;

		Mango::LightingData LightingData;
		Ref<UniformBuffer> LightingUniforms;
		Scope<UniformBuffer> CascadedUniforms;
	};

	struct CSMData {
		xmmatrix MVP[NUM_SHADOW_CASCADES];
	};
	
	static RenderDataLighting* sData;

	void Renderer::InitLighting()
	{
		sData = new RenderDataLighting();

		sData->ShadowSampler = Scope<SamplerState>(SamplerState::Create(SamplerState::Filter::Linear, SamplerState::Address::Border, true));

		sData->LightingShader = Ref<Shader>(Shader::Create("assets/shaders/Lighting_vs.cso", "assets/shaders/Lighting_ps.cso"));
		sData->DirectionalShadowmapShader = Ref<Shader>(Shader::Create("assets/shaders/DirectionalShadowmap_vs.cso", "assets/shaders/DirectionalShadowmap_gs.cso", "assets/shaders/DirectionalShadowmap_ps.cso"));
		
		sData->LightingUniforms = Scope<UniformBuffer>(UniformBuffer::Create<LightingData>());
		sData->CascadedUniforms = Scope<UniformBuffer>(UniformBuffer::Create<CSMData>());

		for (int i = 0; i < MAX_DIRECTIONAL_LIGHTS; i++)
			sData->DirectionalShadowmaps[i] = Ref<CascadedShadowmap>(CascadedShadowmap::Create(1024, 1024, NUM_SHADOW_CASCADES));
	}

	void Renderer::ShutdownLighting()
	{
		delete sData;
	}

	static void FillLightingData() {
		float4x4 proj;
		XMStoreFloat4x4(&proj, Renderer::GetProjectionMatrix() * Renderer::GetJitterMatrix());
		float4 perspectiveValues;
		perspectiveValues.x = 1.0f / proj.m[0][0];
		perspectiveValues.y = 1.0f / proj.m[1][1];
		perspectiveValues.z = proj.m[3][2];
		perspectiveValues.w = -proj.m[2][2];

		sData->LightingData.InvView = XMMatrixInverse(nullptr, Renderer::GetViewMatrix());
		sData->LightingData.PerspectiveValues = perspectiveValues;

		auto ends = CascadedShadowmap::GenerateCascadeDistances(Renderer::GetProjectionMatrix(), NUM_SHADOW_CASCADES);
		for (int i = 0; i < NUM_SHADOW_CASCADES; i++)
			sData->LightingData.CascadeEnds[i] = ends[(size_t)i + 1];
	}

	void Renderer::ShadowmapPass(std::unordered_map<Ref<Material>, std::vector<std::tuple<Ref<VertexArray>, xmmatrix, xmmatrix>>>& queue)
	{
		RenderCommand::ShadowRasterizerState();
		sData->DirectionalShadowmapShader->Bind();
		sData->CascadedUniforms->GSBind(0);

		for (int i = 0; i < sData->LightingData.NumDirectionalLights; i++) {
			auto& shadowmap = sData->DirectionalShadowmaps[i];
			shadowmap->BindAsRenderTarget();
			shadowmap->Clear(0.0f);

			for (auto& [material, submeshes] : queue) {
				for (auto& [va, prevT, transform] : submeshes) {
					CSMData data;
					for (int c = 0; c < NUM_SHADOW_CASCADES; c++)
						data.MVP[c] = transform * sData->LightingData.DirectionalMatrices[i * NUM_SHADOW_CASCADES + c];
					sData->CascadedUniforms->SetData(data);
					va->Bind();
					if (va->IsIndexed())
						RenderCommand::DrawIndexed(va->GetDrawCount(), 0);
					else
						RenderCommand::Draw(va->GetDrawCount(), 0);
				}
			}
		}
		RenderCommand::DefaultRasterizerState();
	}

	void Renderer::LightingPass(const Ref<ColorBuffer>& color, const Ref<ColorBuffer>& normal, const Ref<DepthBuffer>& depth, const Ref<ColorBuffer>& rendertarget)
	{
		BindRenderTargets({ rendertarget });
		rendertarget->Clear(RENDERER_CLEAR_COLOR);

		PointSampler().Bind(0);
		sData->ShadowSampler->Bind(1);
		color->BindAsTexture(0);
		normal->BindAsTexture(1);
		depth->BindAsTexture(2);
		sData->LightingShader->Bind();

		for (int i = 0; i < MAX_DIRECTIONAL_LIGHTS; i++) {
			sData->DirectionalShadowmaps[i]->BindAsTexture((size_t)i + 3);
		}

		FillLightingData();
		sData->LightingUniforms->SetData(sData->LightingData);
		sData->LightingUniforms->PSBind(0);
		DrawScreenQuad();

		sData->LightingData.NumDirectionalLights = 0;
		sData->LightingData.NumPointLights = 0;
	}

	void Renderer::SubmitDirectionalLight(const float3& direction, const float3& color)
	{
		if (sData->LightingData.NumDirectionalLights < MAX_DIRECTIONAL_LIGHTS)
		{
			int index = sData->LightingData.NumDirectionalLights++;
			auto matrices = CascadedShadowmap::GenerateMatrices(direction, GetViewMatrix(), GetProjectionMatrix(), NUM_SHADOW_CASCADES);
			for (int i = 0; i < NUM_SHADOW_CASCADES; i++) {
				sData->LightingData.DirectionalMatrices[index * NUM_SHADOW_CASCADES + i] = matrices[i];
			}
			sData->LightingData.DirectionalLights[index] = { direction, 0.0f, color };
		}
	}

	void Renderer::SubmitPointLight(const float3& position, const float3& color)
	{
		if (sData->LightingData.NumPointLights < MAX_POINT_LIGHTS)
			sData->LightingData.PointLights[sData->LightingData.NumPointLights++] = { position, 0.0f, color };
	}

}