#include "mgpch.h"
#include "RendererPost.h"

#include "Mango/Renderer/Shader.h"

#include "RendererBase.h"

namespace Mango {

	struct RenderDataPost {
		Ref<Shader> TAAShader;
		Ref<Texture> PreviousFrame;
	};

	static RenderDataPost* sData;

	void Renderer::InitPost()
	{
		sData = new RenderDataPost();

		sData->TAAShader = Ref<Shader>(Shader::Create("assets/shaders/Fullscreen_vs.cso", "assets/shaders/TAA_ps.cso"));

		sData->PreviousFrame = Ref<Texture>(Texture::Create(nullptr, 800, 600, Format::RGBA16_FLOAT, Texture_RenderTarget));
	}

	void Renderer::ShutdownPost()
	{
		delete sData;
	}

	void Renderer::TAAPass(const Ref<Texture>& aliasedFrame, const Ref<Texture>& velocityBuffer, const Ref<Texture>& rendertarget)
	{
		sData->PreviousFrame->EnsureSize(aliasedFrame->GetWidth(), aliasedFrame->GetHeight());
		BindRenderTargets({ rendertarget });

		PointSampler().Bind(0);
		sData->TAAShader->Bind();
		aliasedFrame->Bind(0);
		sData->PreviousFrame->Bind(1);
		velocityBuffer->Bind(2);
		DrawScreenQuad();

		BlitTexture(sData->PreviousFrame, rendertarget);
	}

}
