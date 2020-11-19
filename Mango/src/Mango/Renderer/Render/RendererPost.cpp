#include "mgpch.h"
#include "RendererPost.h"

#include "Mango/Renderer/Shader.h"
#include "Mango/Renderer/Buffer.h"

#include "RendererBase.h"

namespace Mango {

	struct SSAOData {
		xmmatrix InvView;
		xmmatrix View;
		xmmatrix Projection;
		float4 PerspectiveValues;
		float randomSeed;
		float3 padding;
	};

	struct RenderDataPost {
		Ref<Shader> TAAShader;
		Ref<Shader> SSAOShader;
		Ref<Shader> AOBlurShader;
		Scope<UniformBuffer> SSAOUniforms;
		Ref<Texture> PreviousFrame;
		Ref<Texture> AOTexture;
	};

	static RenderDataPost* sData;

	void Renderer::InitPost()
	{
		sData = new RenderDataPost();

		sData->TAAShader = Ref<Shader>(Shader::Create("assets/shaders/Fullscreen_vs.cso", "assets/shaders/TAA_ps.cso"));
		sData->SSAOShader = Ref<Shader>(Shader::Create("assets/shaders/Fullscreen_vs.cso", "assets/shaders/SSAO_ps.cso"));
		sData->AOBlurShader = Ref<Shader>(Shader::Create("assets/shaders/Fullscreen_vs.cso", "assets/shaders/AOCombine_ps.cso"));

		sData->SSAOUniforms = Scope<UniformBuffer>(UniformBuffer::Create<SSAOData>());

		sData->PreviousFrame = Ref<Texture>(Texture::Create(nullptr, 800, 600, Format::RGBA16_FLOAT, Texture_RenderTarget));
		sData->AOTexture = Ref<Texture>(Texture::Create(nullptr, 800, 600, Format::RGBA8_UNORM, Texture_RenderTarget));
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

		Texture::Unbind(0);
	}

	void Renderer::SSAOPass(const Ref<DepthBuffer>& depthBuffer, const Ref<Texture>& normalBuffer, const Ref<Texture>& litImage, const Ref<Texture>& rendertarget)
	{
		sData->AOTexture->EnsureSize(rendertarget->GetWidth(), rendertarget->GetHeight());

		BindRenderTargets({ sData->AOTexture });

		static int counter = 0;
		counter++;
		counter = counter % 1000 + 1;

		SSAOData data;
		data.InvView = XMMatrixInverse(nullptr, GetViewMatrix());
		data.View = GetViewMatrix();
		data.Projection = GetProjectionMatrix() * GetJitterMatrix();
		data.randomSeed = (float)counter;
		float4x4 proj;
		XMStoreFloat4x4(&proj, data.Projection);
		data.PerspectiveValues.x = 1.0f / proj.m[0][0];
		data.PerspectiveValues.y = 1.0f / proj.m[1][1];
		data.PerspectiveValues.z = proj.m[3][2];
		data.PerspectiveValues.w = -proj.m[2][2];
		sData->SSAOUniforms->PSBind(0);
		sData->SSAOUniforms->SetData(data);
			
		LinearSamplerClamp().Bind(0);
		sData->SSAOShader->Bind();
		depthBuffer->BindAsTexture(0);
		normalBuffer->Bind(1);
		DrawScreenQuad();

		BindRenderTargets({ rendertarget });
		sData->AOBlurShader->Bind();
		sData->AOTexture->Bind(0);
		litImage->Bind(1);
		LinearSamplerClamp().Bind(0);
		DrawScreenQuad();
		Texture::Unbind(1);
	}

}
