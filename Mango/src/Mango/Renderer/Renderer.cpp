#include "mgpch.h"
#include "Renderer.h"

#include "Mango/Core/Math.h"
#include "Buffer.h"
#include "RenderCommand.h"
#include "Shader.h"

#include "Halton.h"

namespace Mango {

	struct GlobalUniforms {
		xmmatrix PreviousViewProjection;
		xmmatrix ViewProjection;
	};

	struct IndividualData {
		xmmatrix PreviousTransform;
		xmmatrix Transform;
		float4 color;
	};

	struct LightingData {
		xmmatrix InvView;
		float4 PerspectiveValues;
	};

	struct RendererData {
		xmmatrix PreviousView = XMMatrixIdentity();
		Scope<UniformBuffer> GlobalUniforms;
		Scope<UniformBuffer> IndividualUniforms;
		Scope<UniformBuffer> LightingUniforms;

		Ref<VertexArray> Quad;
		Ref<Texture2D> WhiteTexture;
		Scope<SamplerState> SamplerLinear;
		Scope<SamplerState> SamplerPoint;

		Ref<Shader> SpriteShader;
		Ref<Shader> LightingShader;
		Ref<Shader> TAAShader;
		Ref<Shader> GeometryShader;

		Ref<Framebuffer> PreviousFrame;
		Ref<Framebuffer> ImmediateTarget;

		struct {
			Ref<Framebuffer> Color;
			Ref<Framebuffer> Normal;
			Ref<Framebuffer> Velocity;
		} GBuffer;

		bool TAAEnabled = true;

		std::queue<std::tuple<xmmatrix, xmmatrix, Ref<Texture2D>, float4>> RenderQueue2D;
		std::queue<std::tuple<const Mesh*, xmmatrix, xmmatrix>> RenderQueue3D;
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

		FramebufferProperties props;
		props.Width = 800;
		props.Height = 600;
		props.Depth = true;

		sData->GBuffer.Velocity = Ref<Framebuffer>(Framebuffer::Create(props));

		props.Depth = false;
		sData->GBuffer.Normal = Ref<Framebuffer>(Framebuffer::Create(props));
		sData->PreviousFrame = Ref<Framebuffer>(Framebuffer::Create(props));
		sData->GBuffer.Color = Ref<Framebuffer>(Framebuffer::Create(props));
		sData->ImmediateTarget = Ref<Framebuffer>(Framebuffer::Create(props));

		// Textures ----------------------------------------------------------------------------------

		sData->SamplerLinear = Scope<SamplerState>(SamplerState::Create());
		sData->SamplerPoint = Scope<SamplerState>(SamplerState::Create(SamplerState::Mode::Point));

		uint32_t color = 0xffffffff;
		sData->WhiteTexture = Ref<Texture2D>(Texture2D::Create(&color, 1, 1));

		// Uniform Buffers ---------------------------------------------------------------------------

		sData->GlobalUniforms = Scope<UniformBuffer>(UniformBuffer::Create<GlobalUniforms>());
		sData->IndividualUniforms = Scope<UniformBuffer>(UniformBuffer::Create<IndividualData>());
		sData->LightingUniforms = Scope<UniformBuffer>(UniformBuffer::Create<LightingData>());

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
		sData->GlobalUniforms->SetData<GlobalUniforms>({ sData->PreviousView * projection, viewProjection });


		float4x4 proj;
		XMStoreFloat4x4(&proj, projection*jitterMatrix);
		float4 perspectiveValues;
		perspectiveValues.x = 1.0f / proj.m[0][0];
		perspectiveValues.y = 1.0f / proj.m[1][1];
		perspectiveValues.z = proj.m[3][2];
		perspectiveValues.w = -proj.m[2][2];
		sData->LightingUniforms->SetData<LightingData>({XMMatrixInverse(nullptr, view), perspectiveValues});

		sData->PreviousView = view;
	}

	static void InternalDrawQuad(const xmmatrix& previousTransform, const xmmatrix& transform, const Ref<Texture2D>& texture, const float4& color) {
		texture->Bind(0);
		xmmatrix halfScale = XMMatrixScaling(0.5f, 0.5f, 1.0f) * transform;
		xmmatrix halfScalePrev = XMMatrixScaling(0.5f, 0.5f, 1.0f) * previousTransform;
		sData->IndividualUniforms->SetData<IndividualData>({ halfScalePrev, halfScale, color });
		RenderCommand::DrawIndexed(sData->Quad->GetDrawCount(), 0);
	}

	static void InternalRenderNode(const Node* node, const xmmatrix& previousTransform, const xmmatrix& parentTransform) {
		xmmatrix transform = node->Transform * parentTransform;
		xmmatrix prevT = node->Transform * previousTransform;
		sData->IndividualUniforms->SetData<IndividualData>({ prevT, transform, float4(1.0f, 1.0f, 1.0f, 1.0f) });

		for (auto va : node->Submeshes) {
			va->Bind();
			if (va->IsIndexed())
				RenderCommand::DrawIndexed(va->GetDrawCount(), 0);
			else
				RenderCommand::Draw(va->GetDrawCount(), 0);
		}

		for (auto& node : node->Children) {
			InternalRenderNode(&node, prevT, transform);
		}
	}

	void Renderer::EndScene(const Ref<Framebuffer>& target)
	{
		// Render Scene --------------------------------------------------------------------------------
		
		// Initialization
		sData->ImmediateTarget->EnsureSize(target->GetWidth(), target->GetHeight());
		sData->PreviousFrame->EnsureSize(target->GetWidth(), target->GetHeight());
		sData->GBuffer.Color->EnsureSize(target->GetWidth(), target->GetHeight());
		sData->GBuffer.Normal->EnsureSize(target->GetWidth(), target->GetHeight());
		sData->GBuffer.Velocity->EnsureSize(target->GetWidth(), target->GetHeight());
		sData->GBuffer.Velocity->Clear(float4(0.0f, 0.0f, 0.0f, 1.0f));

		Texture::Unbind(0);
		Texture::Unbind(1);
		Texture::Unbind(2);
		sData->GlobalUniforms->VSBind(0);
		sData->IndividualUniforms->VSBind(1);

		// Geometry Pass
		RenderCommand::DisableBlending();
		Framebuffer::BindMultiple({sData->GBuffer.Color, sData->GBuffer.Normal, sData->GBuffer.Velocity});
		sData->GBuffer.Color->Clear(float4(0.0f, 0.0f, 0.0f, 1.0f));
		sData->GBuffer.Normal->Clear(float4(0.0f, 0.0f, 0.0f, 1.0f));
		
		sData->GeometryShader->Bind();
		sData->SamplerLinear->Bind(0);

		while (!sData->RenderQueue3D.empty()) {
			auto& [mesh, previousTransform, transform] = sData->RenderQueue3D.front();
			InternalRenderNode(&mesh->RootNode, previousTransform, transform);
			sData->RenderQueue3D.pop();
		}

		// Lighting
		sData->ImmediateTarget->Bind();
		sData->ImmediateTarget->Clear(RENDERER_CLEAR_COLOR);

		sData->SamplerPoint->Bind(0);
		sData->GBuffer.Color->BindAsTexture(0);
		sData->GBuffer.Normal->BindAsTexture(1);
		sData->GBuffer.Velocity->BindDepthAsTexture(2);
		sData->LightingShader->Bind();
		sData->LightingUniforms->PSBind(0);
		DrawScreenQuad();

		Texture::Unbind(2);
		Framebuffer::BindMultiple({ sData->ImmediateTarget, sData->GBuffer.Velocity });
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

		target->Bind();
		sData->SamplerPoint->Bind(0);
		sData->ImmediateTarget->BindAsTexture(0);
		sData->PreviousFrame->BindAsTexture(1);
		sData->GBuffer.Velocity->BindAsTexture(2);
		sData->TAAShader->Bind();
		DrawScreenQuad();

		Framebuffer::Blit(sData->PreviousFrame, target);

		// -------------------------------------------------------------------------------------------
	}

	void Renderer::DrawQuad(const xmmatrix& previousFrameTransform, const xmmatrix& transform, const float4& color)
	{
		sData->RenderQueue2D.push({ previousFrameTransform, transform, sData->WhiteTexture, color });
	}

	void Renderer::DrawQuad(const xmmatrix& previousFrameTransform, const xmmatrix& transform, const Ref<Texture2D>& texture)
	{
		sData->RenderQueue2D.push({ previousFrameTransform, transform, texture, float4(1.0f, 1.0f, 1.0f, 1.0f) });
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