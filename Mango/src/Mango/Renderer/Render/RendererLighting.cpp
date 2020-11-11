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
#define SHADOW_RESOLUTION 1024

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
		float EnvironmentStrength = 1.0f;
		float padding;
		float4 PerspectiveValues;
		float CascadeEnds[NUM_SHADOW_CASCADES];
	};

	struct RenderToCubemapMatrices {
		xmmatrix MVP[6];
	};

	struct SkyboxData {
		xmmatrix ViewMatrix;
		xmmatrix ProjectionMatrix;
		float EnvironmentStrength;
		float3 padding;
	};

	struct RenderDataLighting {
		Ref<SamplerState> ShadowSampler;
		Ref<CascadedShadowmap> DirectionalShadowmaps[MAX_DIRECTIONAL_LIGHTS];

		Ref<Shader> LightingShader;
		Ref<Shader> DirectionalShadowmapShader;
		Ref<Shader> SkyboxShader;
		Ref<Shader> EquiToCubemapShader;
		Ref<Shader> ConvolutionShader;
		Ref<Shader> PrefilterShader;

		Ref<Cubemap> Skybox;
		Ref<Cubemap> IrradianceMap;
		Ref<Cubemap> PrefilteredCubemap;
		Ref<ColorBuffer> BRDFLUT;

		Mango::LightingData LightingData;
		Ref<UniformBuffer> LightingUniforms;
		Scope<UniformBuffer> CascadedUniforms;
		Scope<UniformBuffer> SkyboxUniforms;
		Scope<UniformBuffer> RenderToCubemapUniforms;
		Scope<UniformBuffer> PrefilterUniforms;

		Ref<VertexArray> CubeVA;
	};

	struct CSMData {
		xmmatrix MVP[NUM_SHADOW_CASCADES];
	};
	
	static RenderDataLighting* sData;

	static void CreateBRDFLookup() {
		auto shader = Scope<Shader>(Shader::Create("assets/shaders/Fullscreen_vs.cso", "assets/shaders/BRDF_ps.cso"));
		
		BindRenderTargets({ sData->BRDFLUT });
		sData->BRDFLUT->Clear(RENDERER_CLEAR_COLOR);
		shader->Bind();
		Renderer::DrawScreenQuad();
	}

	void Renderer::InitLighting()
	{
		sData = new RenderDataLighting();

		sData->ShadowSampler = Scope<SamplerState>(SamplerState::Create(SamplerState::Filter::Linear, SamplerState::Address::Border, true));

		sData->LightingShader = Ref<Shader>(Shader::Create("assets/shaders/Fullscreen_vs.cso", "assets/shaders/Lighting_ps.cso"));
		sData->DirectionalShadowmapShader = Ref<Shader>(Shader::Create("assets/shaders/DirectionalShadowmap_vs.cso", "assets/shaders/DirectionalShadowmap_gs.cso", "assets/shaders/DirectionalShadowmap_ps.cso"));
		sData->SkyboxShader = Ref<Shader>(Shader::Create("assets/shaders/Skybox_vs.cso", "assets/shaders/Skybox_ps.cso"));
		sData->EquiToCubemapShader = Ref<Shader>(Shader::Create("assets/shaders/RenderToCubemap_vs.cso", "assets/shaders/RenderToCubemap_gs.cso", "assets/shaders/EquiToCubemap_ps.cso"));
		sData->ConvolutionShader = Ref<Shader>(Shader::Create("assets/shaders/RenderToCubemap_vs.cso", "assets/shaders/RenderToCubemap_gs.cso", "assets/shaders/Convolution_ps.cso"));
		sData->PrefilterShader = Ref<Shader>(Shader::Create("assets/shaders/RenderToCubemap_vs.cso", "assets/shaders/RenderToCubemap_gs.cso", "assets/shaders/CubemapPrefilter_ps.cso"));

		sData->LightingUniforms = Scope<UniformBuffer>(UniformBuffer::Create<LightingData>());
		sData->CascadedUniforms = Scope<UniformBuffer>(UniformBuffer::Create<CSMData>());
		sData->SkyboxUniforms = Scope<UniformBuffer>(UniformBuffer::Create<SkyboxData>());
		sData->RenderToCubemapUniforms = Scope<UniformBuffer>(UniformBuffer::Create<RenderToCubemapMatrices>());
		sData->PrefilterUniforms = Scope<UniformBuffer>(UniformBuffer::Create<float4>());

		xmmatrix captureProjection = XMMatrixPerspectiveFovLH(ToRadians(90.0f), 1.0f, 10.0f, 0.1f);
		RenderToCubemapMatrices hdriData;
		hdriData.MVP[0] = XMMatrixLookAtLH({ 0.0f, 0.0f, 0.0f }, {  1.0f,  0.0f,  0.0f }, { 0.0f,  1.0f, 0.0f }) * captureProjection;
		hdriData.MVP[1] = XMMatrixLookAtLH({ 0.0f, 0.0f, 0.0f }, { -1.0f,  0.0f,  0.0f }, { 0.0f,  1.0f, 0.0f }) * captureProjection;
		hdriData.MVP[2] = XMMatrixLookAtLH({ 0.0f, 0.0f, 0.0f }, {  0.0f,  1.0f,  0.0f }, { 0.0f,  0.0f,-1.0f }) * captureProjection;
		hdriData.MVP[3] = XMMatrixLookAtLH({ 0.0f, 0.0f, 0.0f }, {  0.0f, -1.0f,  0.0f }, { 0.0f,  0.0f, 1.0f }) * captureProjection;
		hdriData.MVP[4] = XMMatrixLookAtLH({ 0.0f, 0.0f, 0.0f }, {  0.0f,  0.0f,  1.0f }, { 0.0f,  1.0f, 0.0f }) * captureProjection;
		hdriData.MVP[5] = XMMatrixLookAtLH({ 0.0f, 0.0f, 0.0f }, {  0.0f,  0.0f, -1.0f }, { 0.0f,  1.0f, 0.0f }) * captureProjection;

		sData->RenderToCubemapUniforms->SetData(hdriData);
		sData->IrradianceMap = Ref<Cubemap>(Cubemap::Create("", 32));
		sData->PrefilteredCubemap = Ref<Cubemap>(Cubemap::Create("", 512));

		ColorBufferProperties props;
		props.Width = 512;
		props.Height = 512;
		props.Format = Format::RGBA16_FLOAT;
		sData->BRDFLUT = Ref<ColorBuffer>(ColorBuffer::Create(props));
		CreateBRDFLookup();

		for (int i = 0; i < MAX_DIRECTIONAL_LIGHTS; i++)
			sData->DirectionalShadowmaps[i] = Ref<CascadedShadowmap>(CascadedShadowmap::Create(SHADOW_RESOLUTION, SHADOW_RESOLUTION, NUM_SHADOW_CASCADES));

		float vertices[] = {
			-0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  0.0f, 1.0f,

			-0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
			-0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

			-0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

			 0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
			 0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
			 0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,  0.0f, 0.0f,


			-0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
			 0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
			-0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
			-0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

			-0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
			-0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,  0.0f, 1.0f
		};

		auto vb = Ref<VertexBuffer>(VertexBuffer::Create(vertices, std::size(vertices) / 8, 8 * sizeof(float)));

		sData->CubeVA = CreateRef<VertexArray>(vb, Ref<IndexBuffer>());
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
		RenderCommand::DisableCulling();
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
		RenderCommand::EnableCulling();
	}

	void Renderer::LightingPass(const Ref<ColorBuffer>& color, const Ref<ColorBuffer>& normal, const Ref<DepthBuffer>& depth, const Ref<ColorBuffer>& rendertarget)
	{
		BindRenderTargets({ rendertarget });
		rendertarget->Clear(RENDERER_CLEAR_COLOR);

		if (sData->Skybox)
		{
			LinearSampler().Bind(0);
			sData->Skybox->Bind(0);
			sData->SkyboxUniforms->SetData<SkyboxData>({ GetViewMatrix(), GetProjectionMatrix(), sData->LightingData.EnvironmentStrength });
			sData->SkyboxUniforms->VSBind(0);
			sData->SkyboxShader->Bind();

			sData->CubeVA->Bind();
			RenderCommand::DisableCulling();
			RenderCommand::Draw(sData->CubeVA->GetDrawCount(), 0);
			RenderCommand::EnableCulling();
		}

		PointSampler().Bind(0);
		LinearSampler().Bind(1);
		LinearSamplerClamp().Bind(2);
		sData->ShadowSampler->Bind(3);
		color->BindAsTexture(0);
		normal->BindAsTexture(1);
		depth->BindAsTexture(2);
		sData->IrradianceMap->Bind(8);
		sData->PrefilteredCubemap->Bind(9);
		sData->BRDFLUT->BindAsTexture(10);
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
			auto matrices = CascadedShadowmap::GenerateMatrices(direction, GetViewMatrix(), GetProjectionMatrix(), NUM_SHADOW_CASCADES, SHADOW_RESOLUTION);
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

	void Renderer::SetSkybox(const Ref<Cubemap>& skybox)
	{
		sData->Skybox = skybox;

		RenderCommand::DisableCulling();

		Texture::Unbind(8);
		Texture::Unbind(9);

		// Irradiance
		sData->IrradianceMap->BindAsRenderTarget();
		sData->RenderToCubemapUniforms->GSBind(0);
		sData->ConvolutionShader->Bind();
		skybox->Bind(0);
		LinearSampler().Bind(0);
		sData->CubeVA->Bind();
		RenderCommand::Draw(sData->CubeVA->GetDrawCount(), 0);

		// Prefiltering
		sData->RenderToCubemapUniforms->GSBind(0);
		sData->PrefilterUniforms->PSBind(0);
		sData->PrefilterShader->Bind();
		skybox->Bind(0);
		LinearSampler().Bind(0);
		sData->CubeVA->Bind();
		uint32_t mipLevels = sData->PrefilteredCubemap->GetMipLevels();
		for (uint32_t i = 0; i < mipLevels; i++) {
			sData->PrefilteredCubemap->BindAsRenderTarget(i);
			float roughness = (float)i / ((float)mipLevels - 1.0f);
			sData->PrefilterUniforms->SetData(float4(roughness, 0.0f, 0.0f, 0.0f));
			RenderCommand::Draw(sData->CubeVA->GetDrawCount(), 0);
		}

		RenderCommand::EnableCulling();
	}

	const Ref<Cubemap>& Renderer::GetSkybox()
	{
		return sData->Skybox;
	}

	float& Renderer::EnvironmentStrength()
	{
		return sData->LightingData.EnvironmentStrength;
	}

	void Renderer::InitializeCubemap(const Ref<Cubemap>& cubemap)
	{
		Scope<Texture2D> hdri = Scope<Texture2D>(Texture2D::Create(cubemap->GetPath(), Format::RGBA32_FLOAT));

		RenderCommand::DisableCulling();

		cubemap->BindAsRenderTarget();
		sData->RenderToCubemapUniforms->GSBind(0);
		sData->EquiToCubemapShader->Bind();
		hdri->Bind(0);
		LinearSampler().Bind(0);
		sData->CubeVA->Bind();
		RenderCommand::Draw(sData->CubeVA->GetDrawCount(), 0);

		cubemap->GenerateMips();

		RenderCommand::EnableCulling();
	}

}