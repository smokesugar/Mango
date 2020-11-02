#include "mgpch.h"
#include "Renderer.h"

#include "Mango/Core/Math.h"
#include "Buffer.h"
#include "RenderCommand.h"
#include "Shader.h"

#include "Halton.h"

#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 16

namespace Mango {

	struct GlobalData {
		xmmatrix PreviousViewProjection;
		xmmatrix ViewProjection;
	};

	struct IndividualData {
		xmmatrix PreviousTransform;
		xmmatrix Transform;
		float4 color;
	};

	struct Light {
		float3 Vector;
		float padding0;
		float3 Color;
		float padding1;
	};

	struct LightingData {
		xmmatrix InvView;
		float4 PerspectiveValues;
		Light PointLights[MAX_POINT_LIGHTS];
		Light DirectionalLights[MAX_DIRECTIONAL_LIGHTS];
		int NumDirectionalLights;
		int NumPointLights;
	};

	struct SurfaceData {
		float3 AlbedoColor;
		int UseNormalMap;
		float RoughnessValue;
		float Metalness;
		float2 _padding;
	};

	struct RendererData {
		xmmatrix PreviousViewProjection = XMMatrixIdentity();
		Scope<UniformBuffer> GlobalUniforms;
		Scope<UniformBuffer> IndividualUniforms;
		Mango::LightingData LightingData;
		Scope<UniformBuffer> LightingUniforms;
		Scope<UniformBuffer> SurfaceUniforms;

		Ref<VertexArray> Quad;
		Ref<Texture2D> WhiteTexture;
		Ref<Texture2D> BlackTexture;
		Scope<SamplerState> SamplerLinear;
		Scope<SamplerState> SamplerPoint;

		Ref<Shader> SpriteShader;
		Ref<Shader> LightingShader;
		Ref<Shader> TAAShader;
		Ref<Shader> GeometryShader;

		Ref<ColorBuffer> PreviousFrame;
		Ref<ColorBuffer> ImmediateTarget;
		Ref<Mango::DepthBuffer> DepthBuffer;

		struct {
			Ref<ColorBuffer> Color;
			Ref<ColorBuffer> Normal;
			Ref<ColorBuffer> Velocity;
		} GBuffer;

		bool TAAEnabled = true;

		std::queue<std::tuple<xmmatrix, xmmatrix, Ref<Texture2D>, float4>> RenderQueue2D;
		std::queue<std::tuple<const Mesh*, xmmatrix, xmmatrix>> RenderQueue3D;
		std::unordered_map<Ref<Material>, std::vector<std::tuple<Ref<VertexArray>, xmmatrix, xmmatrix>>> MaterialMeshQueue;
	};

	static RendererData* sData;

	void Renderer::Init()
	{
		RenderCommand::EnableInvertedDepthTesting();

		sData = new RendererData();

		// Shaders -----------------------------------------------------------------------------------

		sData->SpriteShader = Ref<Shader>(Shader::Create("assets/shaders/Renderer2D_vs.cso", "assets/shaders/Renderer2D_ps.cso"));
		sData->LightingShader = Ref<Shader>(Shader::Create("assets/shaders/Lighting_vs.cso", "assets/shaders/Lighting_ps.cso"));
		sData->TAAShader = Ref<Shader>(Shader::Create("assets/shaders/TAA_vs.cso", "assets/shaders/TAA_ps.cso"));
		sData->GeometryShader = Ref<Shader>(Shader::Create("assets/shaders/GeometryPass_vs.cso", "assets/shaders/GeometryPass_ps.cso"));

		// Framebuffers ------------------------------------------------------------------------------

		ColorBufferProperties props;
		props.Width = 800;
		props.Height = 600;
		props.Format = Format::RGBA16_FLOAT;

		sData->GBuffer.Velocity = Ref<ColorBuffer>(ColorBuffer::Create(props));
		sData->GBuffer.Normal = Ref<ColorBuffer>(ColorBuffer::Create(props));
		sData->PreviousFrame = Ref<ColorBuffer>(ColorBuffer::Create(props));
		sData->GBuffer.Color = Ref<ColorBuffer>(ColorBuffer::Create(props));
		sData->ImmediateTarget = Ref<ColorBuffer>(ColorBuffer::Create(props));

		sData->DepthBuffer = Ref<DepthBuffer>(DepthBuffer::Create(800, 600));

		// Textures ----------------------------------------------------------------------------------

		sData->SamplerLinear = Scope<SamplerState>(SamplerState::Create());
		sData->SamplerPoint = Scope<SamplerState>(SamplerState::Create(SamplerState::Mode::Point));

		uint32_t color = 0xffffffff;
		sData->WhiteTexture = Ref<Texture2D>(Texture2D::Create(&color, 1, 1));
		color = 0;
		sData->BlackTexture = Ref<Texture2D>(Texture2D::Create(&color, 1, 1));

		// Uniform Buffers ---------------------------------------------------------------------------

		sData->GlobalUniforms = Scope<UniformBuffer>(UniformBuffer::Create<GlobalData>());
		sData->IndividualUniforms = Scope<UniformBuffer>(UniformBuffer::Create<IndividualData>());
		sData->LightingUniforms = Scope<UniformBuffer>(UniformBuffer::Create<LightingData>());
		sData->SurfaceUniforms = Scope<UniformBuffer>(UniformBuffer::Create<SurfaceData>());

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

	const Ref<Texture2D>& Renderer::GetWhiteTexture()
	{
		return sData->WhiteTexture;
	}

	const Ref<Texture2D>& Renderer::GetBlackTexture()
	{
		return sData->BlackTexture;
	}

	Ref<Material> Renderer::CreateDefaultMaterial()
	{
		return CreateRef<Material>(Renderer::GetWhiteTexture(), nullptr, Renderer::GetWhiteTexture(), float3(1.0f, 1.0f, 1.0f), 0.5f, 0.1f);
	}

	void Renderer::BeginScene(const xmmatrix& projection, const xmmatrix& transform, uint32_t width, uint32_t height)
	{
		static size_t frame = 0;
		frame++;

		if (frame >= 10000)
			frame = 0;

		float4 halt = GetHaltonSequence(frame);

		float xOffset = (halt.x * 2.0f - 1.0f);
		float yOffset = (halt.y * 2.0f - 1.0f);

		float xJit = xOffset / (float)width;
		float yJit = yOffset / (float)height;

		xmmatrix jitterMatrix = sData->TAAEnabled ? XMMatrixTranslation(xJit, yJit, 0.0f) : XMMatrixIdentity();

		xmmatrix view = XMMatrixInverse(nullptr, transform);
		xmmatrix viewProjection = view * projection * jitterMatrix;
		sData->GlobalUniforms->SetData<GlobalData>({ sData->PreviousViewProjection * jitterMatrix, viewProjection });

		float4x4 proj;
		XMStoreFloat4x4(&proj, projection*jitterMatrix);
		float4 perspectiveValues;
		perspectiveValues.x = 1.0f / proj.m[0][0];
		perspectiveValues.y = 1.0f / proj.m[1][1];
		perspectiveValues.z = proj.m[3][2];
		perspectiveValues.w = -proj.m[2][2];

		sData->LightingData.InvView = XMMatrixInverse(nullptr, view);
		sData->LightingData.PerspectiveValues = perspectiveValues;
		sData->LightingData.NumDirectionalLights = 0;
		sData->LightingData.NumPointLights = 0;

		sData->PreviousViewProjection = view * projection;
	}

	static void InternalDrawQuad(const xmmatrix& previousTransform, const xmmatrix& transform, const Ref<Texture2D>& texture, const float4& color) {
		texture->Bind(0);
		xmmatrix halfScale = XMMatrixScaling(0.5f, 0.5f, 1.0f) * transform;
		xmmatrix halfScalePrev = XMMatrixScaling(0.5f, 0.5f, 1.0f) * previousTransform;
		sData->IndividualUniforms->SetData<IndividualData>({ halfScalePrev, halfScale, color });
		RenderCommand::DrawIndexed(sData->Quad->GetDrawCount(), 0);
	}

	static void SubmitNode(const Node* node, const xmmatrix& previousTransform, const xmmatrix& parentTransform) {
		xmmatrix transform = node->Transform * parentTransform;
		xmmatrix prevT = node->Transform * previousTransform;

		for (auto& [va, material] : node->Submeshes)
		{
			sData->MaterialMeshQueue[material].push_back({va, prevT, transform});
		}

		for (auto& node : node->Children) {
			SubmitNode(&node, prevT, transform);
		}
	}

	void Renderer::EndScene(const Ref<ColorBuffer>& target)
	{
		// Render Scene --------------------------------------------------------------------------------
		
		// Initialization
		sData->DepthBuffer->EnsureSize(target->GetWidth(), target->GetHeight());
		sData->ImmediateTarget->EnsureSize(target->GetWidth(), target->GetHeight());
		sData->PreviousFrame->EnsureSize(target->GetWidth(), target->GetHeight());
		sData->GBuffer.Color->EnsureSize(target->GetWidth(), target->GetHeight());
		sData->GBuffer.Normal->EnsureSize(target->GetWidth(), target->GetHeight());
		sData->GBuffer.Velocity->EnsureSize(target->GetWidth(), target->GetHeight());

		Texture::Unbind(0);
		Texture::Unbind(1);
		Texture::Unbind(2);
		sData->GlobalUniforms->VSBind(0);
		sData->IndividualUniforms->VSBind(1);

		// Geometry Pass ----------------------------------------------------------------------------------

		RenderCommand::DisableBlending();
		BindRenderTargets({sData->GBuffer.Color, sData->GBuffer.Normal, sData->GBuffer.Velocity}, sData->DepthBuffer);
		sData->GBuffer.Color->Clear(float4(0.0f, 0.0f, 0.0f, 1.0f));
		sData->GBuffer.Normal->Clear(float4(0.0f, 0.0f, 0.0f, 1.0f));
		sData->GBuffer.Velocity->Clear(float4(0.0f, 0.0f, 0.0f, 1.0f));
		sData->DepthBuffer->Clear(0.0f);
		
		sData->GeometryShader->Bind();
		sData->SamplerLinear->Bind(0);
		sData->SurfaceUniforms->PSBind(0);

		while (!sData->RenderQueue3D.empty()) {
			auto& [mesh, previousTransform, transform] = sData->RenderQueue3D.front();
			SubmitNode(&mesh->RootNode, previousTransform, transform);

			for (auto& [material, submeshes] : sData->MaterialMeshQueue) {
				sData->SurfaceUniforms->SetData<SurfaceData>({ material->AlbedoColor, material->NormalTexture ? true : false, material->RoughnessValue, material->Metalness });
				material->AlbedoTexture->Bind(0);
				if (material->NormalTexture)
					material->NormalTexture->Bind(1);
				material->RoughnessTexture->Bind(2);

				for (auto& [va, prevT, transform] : submeshes) {
					sData->IndividualUniforms->SetData<IndividualData>({ prevT, transform, float4(1.0f, 1.0f, 1.0f, 1.0f) });

					va->Bind();
					if (va->IsIndexed())
						RenderCommand::DrawIndexed(va->GetDrawCount(), 0);
					else
						RenderCommand::Draw(va->GetDrawCount(), 0);
				}
			}

			sData->MaterialMeshQueue.clear();
			sData->RenderQueue3D.pop();
		}

		// Lighting ---------------------------------------------------------------------------

		BindRenderTargets({sData->ImmediateTarget});
		sData->ImmediateTarget->Clear(RENDERER_CLEAR_COLOR);

		sData->SamplerPoint->Bind(0);
		sData->GBuffer.Color->BindAsTexture(0);
		sData->GBuffer.Normal->BindAsTexture(1);
		sData->DepthBuffer->BindAsTexture(2);
		sData->LightingShader->Bind();
		sData->LightingUniforms->SetData(sData->LightingData);
		sData->LightingUniforms->PSBind(0);
		DrawScreenQuad();

		// Sprites ------------------------------------------------------------------------------

		Texture::Unbind(2);
		BindRenderTargets({ sData->ImmediateTarget, sData->GBuffer.Velocity }, sData->DepthBuffer);
		RenderCommand::EnableBlending();
		sData->SpriteShader->Bind();
		sData->SamplerLinear->Bind(0);
		sData->Quad->Bind();
		while (!sData->RenderQueue2D.empty()) {
			auto& [prevTransform, transform, texture, color] = sData->RenderQueue2D.front();
			InternalDrawQuad(prevTransform, transform, texture, color);
			sData->RenderQueue2D.pop();
		}

		// Temporal Anti-aliasing --------------------------------------------------------------------

		BindRenderTargets({target});
		sData->SamplerPoint->Bind(0);
		sData->ImmediateTarget->BindAsTexture(0);
		sData->PreviousFrame->BindAsTexture(1);
		sData->GBuffer.Velocity->BindAsTexture(2);
		sData->TAAShader->Bind();
		DrawScreenQuad();

		ColorBuffer::Blit(sData->PreviousFrame, target);

		// -------------------------------------------------------------------------------------------
	}

	void Renderer::SubmitDirectionalLight(const float3& direction, const float3& color)
	{
		if (sData->LightingData.NumDirectionalLights < MAX_DIRECTIONAL_LIGHTS)
			sData->LightingData.DirectionalLights[sData->LightingData.NumDirectionalLights++] = { direction, 0.0f, color };
	}

	void Renderer::SubmitPointLight(const float3& position, const float3& color)
	{
		if(sData->LightingData.NumPointLights < MAX_POINT_LIGHTS)
			sData->LightingData.PointLights[sData->LightingData.NumPointLights++] = { position, 0.0f, color };
	}

	void Renderer::SubmitQuad(const xmmatrix& previousFrameTransform, const xmmatrix& transform, const Ref<Texture2D>& texture, const float4& color)
	{
		sData->RenderQueue2D.push({ previousFrameTransform, transform, texture, color });
	}

	void Renderer::DrawScreenQuad()
	{
		sData->Quad->Bind();
		RenderCommand::DrawIndexed(sData->Quad->GetDrawCount(), 0);
	}

	void Renderer::SubmitMesh(const Mesh& mesh, const xmmatrix& previousFrameTransform, const xmmatrix& transform)
	{
		sData->RenderQueue3D.push({ &mesh, previousFrameTransform, transform });
	}

}