#include "mgpch.h"
#include "RendererPost.h"

#include "Mango/Renderer/Shader.h"

#include "RendererBase.h"

namespace Mango {

	struct RenderDataPost {
		Ref<Shader> TAAShader;
		Ref<ColorBuffer> PreviousFrame;
	};

	static RenderDataPost* sData;

	void Renderer::InitPost()
	{
		sData = new RenderDataPost();

		sData->TAAShader = Ref<Shader>(Shader::Create("assets/shaders/Fullscreen_vs.cso", "assets/shaders/TAA_ps.cso"));

		ColorBufferProperties props;
		props.Width = 800;
		props.Height = 600;
		props.Format = Format::RGBA16_FLOAT;
		sData->PreviousFrame = Ref<ColorBuffer>(ColorBuffer::Create(props));
	}

	void Renderer::ShutdownPost()
	{
		delete sData;
	}

	void Renderer::TAAPass(const Ref<ColorBuffer>& aliasedFrame, const Ref<ColorBuffer>& velocityBuffer, const Ref<ColorBuffer>& rendertarget)
	{
		sData->PreviousFrame->EnsureSize(aliasedFrame->GetWidth(), aliasedFrame->GetHeight());
		BindRenderTargets({ rendertarget });

		PointSampler().Bind(0);
		sData->TAAShader->Bind();
		aliasedFrame->BindAsTexture(0);
		sData->PreviousFrame->BindAsTexture(1);
		velocityBuffer->BindAsTexture(2);
		DrawScreenQuad();

		ColorBuffer::Blit(sData->PreviousFrame, rendertarget);
	}

}
