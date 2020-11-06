#include "mgpch.h"
#include "Renderer.h"

#include "Mango/Core/Math.h"
#include "Mango/Renderer/Buffer.h"
#include "Mango/Renderer/RenderCommand.h"
#include "Mango/Renderer/Shader.h"
#include "Mango/Renderer/CascadedShadowmap.h"
#include "Mango/Renderer/Halton.h"

#include "RendererBase.h"
#include "RendererSprite.h"
#include "RendererGeometry.h"
#include "RendererPost.h"

namespace Mango {

	struct RendererData {
		Ref<Texture2D> WhiteTexture;
		Ref<Texture2D> BlackTexture;

		struct {
			Ref<ColorBuffer> Color;
			Ref<ColorBuffer> Normal;
			Ref<ColorBuffer> Velocity;
		} GBuffer;
		Ref<ColorBuffer> ImmediateTarget;
		Ref<Mango::DepthBuffer> DepthBuffer;

		bool TAAEnabled = true;

		std::queue<std::tuple<xmmatrix, xmmatrix, Ref<Texture2D>, float4>> RenderQueue2D;
		std::unordered_map<Ref<Material>, std::vector<std::tuple<Ref<VertexArray>, xmmatrix, xmmatrix>>> RenderQueue3D;
	};

	static RendererData* sData;

	void Renderer::Init()
	{
		RenderCommand::EnableInvertedDepthTesting();

		Renderer::InitBase();
		Renderer::InitSprite();
		Renderer::InitGeometry();
		Renderer::InitLighting();
		Renderer::InitPost();
		sData = new RendererData();

		// Framebuffers ------------------------------------------------------------------------------

		ColorBufferProperties props;
		props.Width = 800;
		props.Height = 600;
		props.Format = Format::RGBA16_FLOAT;

		sData->GBuffer.Velocity = Ref<ColorBuffer>(ColorBuffer::Create(props));
		sData->GBuffer.Normal = Ref<ColorBuffer>(ColorBuffer::Create(props));
		sData->GBuffer.Color = Ref<ColorBuffer>(ColorBuffer::Create(props));
		sData->ImmediateTarget = Ref<ColorBuffer>(ColorBuffer::Create(props));

		sData->DepthBuffer = Ref<DepthBuffer>(DepthBuffer::Create(800, 600));

		// Textures ----------------------------------------------------------------------------------

		uint32_t color = 0xffffffff;
		sData->WhiteTexture = Ref<Texture2D>(Texture2D::Create(&color, 1, 1));
		color = 0;
		sData->BlackTexture = Ref<Texture2D>(Texture2D::Create(&color, 1, 1));
	}

	void Renderer::Shutdown()
	{
		Renderer::ShutdownBase();
		Renderer::ShutdownSprite();
		Renderer::ShutdownGeometry();
		Renderer::ShutdownLighting();
		Renderer::ShutdownPost();
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
		StoreViewMatrix(XMMatrixInverse(nullptr, transform));
		StoreProjectionMatrix(projection);

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
		StoreJitterMatrix(jitterMatrix);

		sData->RenderQueue3D.clear();
	}

	static void SubmitNode(const Node* node, const xmmatrix& previousTransform, const xmmatrix& parentTransform) {
		xmmatrix transform = node->Transform * parentTransform;
		xmmatrix prevT = node->Transform * previousTransform;

		for (auto& [va, material] : node->Submeshes)
		{
			sData->RenderQueue3D[material].push_back({va, prevT, transform});
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
		sData->GBuffer.Color->EnsureSize(target->GetWidth(), target->GetHeight());
		sData->GBuffer.Normal->EnsureSize(target->GetWidth(), target->GetHeight());
		sData->GBuffer.Velocity->EnsureSize(target->GetWidth(), target->GetHeight());

		Texture::Unbind(0);
		Texture::Unbind(1);
		Texture::Unbind(2);
		Texture::Unbind(3);
		Texture::Unbind(4);
		Texture::Unbind(5);
		Texture::Unbind(6);
		Texture::Unbind(7);

		ShadowmapPass(sData->RenderQueue3D);
		RenderToGBuffer(sData->RenderQueue3D, {sData->GBuffer.Color, sData->GBuffer.Normal, sData->GBuffer.Velocity}, sData->DepthBuffer);
		LightingPass(sData->GBuffer.Color, sData->GBuffer.Normal, sData->DepthBuffer, sData->ImmediateTarget);
		Texture::Unbind(2);
		FlushSpriteQueue(sData->RenderQueue2D, { sData->ImmediateTarget, sData->GBuffer.Velocity }, sData->DepthBuffer);
		TAAPass(sData->ImmediateTarget, sData->GBuffer.Velocity, target);
	}

	void Renderer::SubmitQuad(const xmmatrix& previousFrameTransform, const xmmatrix& transform, const Ref<Texture2D>& texture, const float4& color)
	{
		sData->RenderQueue2D.push({ previousFrameTransform, transform, texture, color });
	}

	void Renderer::SubmitMesh(const Mesh& mesh, const xmmatrix& previousFrameTransform, const xmmatrix& transform)
	{
		SubmitNode(&mesh.RootNode, previousFrameTransform, transform);
	}

}