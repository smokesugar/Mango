#include "mgpch.h"
#include "RendererPost.h"

#include <random>

#include "Mango/Renderer/Shader.h"
#include "Mango/Renderer/Buffer.h"

#include "RendererBase.h"

namespace Mango {

	struct SSAOData {
		xmmatrix View;
		xmmatrix Projection;
		float4 PerspectiveValues;
		float4 randomHemispheres[64];
		float randomSeed;
		float radius = 1.0f;
		float2 padding;
	};

	struct RenderDataPost {
		Ref<Shader> TAAShader;
		Ref<Shader> SSAOShader;
		Ref<Shader> AOBlurShader;

		SSAOData SSAOCbuffer;
		Scope<UniformBuffer> SSAOUniforms;

		Ref<Texture> PreviousFrame;
		Ref<Texture> AOTexture;
	};

	static RenderDataPost* sData;

	static float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	void Renderer::InitPost()
	{
		sData = new RenderDataPost();

		sData->TAAShader = Ref<Shader>(Shader::Create("assets/shaders/Fullscreen_vs.cso", "assets/shaders/TAA_ps.cso"));
		sData->SSAOShader = Ref<Shader>(Shader::Create("assets/shaders/Fullscreen_vs.cso", "assets/shaders/SSAO_ps.cso"));
		sData->AOBlurShader = Ref<Shader>(Shader::Create("assets/shaders/Fullscreen_vs.cso", "assets/shaders/AOCombine_ps.cso"));

		sData->SSAOUniforms = Scope<UniformBuffer>(UniformBuffer::Create<SSAOData>());

		sData->PreviousFrame = Ref<Texture>(Texture::Create(nullptr, 800, 600, Format::RGBA16_FLOAT, Texture_RenderTarget));
		sData->AOTexture = Ref<Texture>(Texture::Create(nullptr, 800, 600, Format::RGBA8_UNORM, Texture_RenderTarget));

		static std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
		static std::default_random_engine generator;
		for (int i = 0; i < 64; i++) {
			xmvector sample = { randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) };
			sample = XMVector3Normalize(sample);
			sample *= randomFloats(generator);
			float scale = (float)i / 64.0f;
			scale = lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			XMStoreFloat4(&sData->SSAOCbuffer.randomHemispheres[i], sample);
		}
	}

	void Renderer::ShutdownPost()
	{
		delete sData;
	}

	void Renderer::TAAPass(const Ref<Texture>& aliasedFrame, const Ref<Texture>& velocityBuffer, const Ref<Texture>& rendertarget)
	{
		if (sData->PreviousFrame->GetFormat() != rendertarget->GetFormat()) {
			sData->PreviousFrame = Ref<Texture>(Texture::Create(nullptr, 1, 1, rendertarget->GetFormat(), Texture_RenderTarget));
		}
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

	void Renderer::SSAOPass(const Ref<DepthBuffer>& depthBuffer, const Ref<Texture>& normalBuffer, const Ref<Texture>& rendertarget)
	{
		sData->AOTexture->EnsureSize(rendertarget->GetWidth(), rendertarget->GetHeight());

		BindRenderTargets({ sData->AOTexture });

		static int counter = 0;
		counter++;
		counter = counter % 1000 + 1;

		sData->SSAOCbuffer.View = GetViewMatrix();
		sData->SSAOCbuffer.Projection = GetProjectionMatrix();
		sData->SSAOCbuffer.randomSeed = (float)counter;
		float4x4 proj;
		XMStoreFloat4x4(&proj, sData->SSAOCbuffer.Projection);
		sData->SSAOCbuffer.PerspectiveValues.x = 1.0f / proj.m[0][0];
		sData->SSAOCbuffer.PerspectiveValues.y = 1.0f / proj.m[1][1];
		sData->SSAOCbuffer.PerspectiveValues.z = proj.m[3][2];
		sData->SSAOCbuffer.PerspectiveValues.w = -proj.m[2][2];

		sData->SSAOUniforms->PSBind(0);
		sData->SSAOUniforms->SetData(sData->SSAOCbuffer);
			
		LinearSamplerClamp().Bind(0);
		sData->SSAOShader->Bind();
		depthBuffer->BindAsTexture(0);
		normalBuffer->Bind(1);
		DrawScreenQuad();

		BindRenderTargets({ rendertarget });
		sData->AOBlurShader->Bind();
		sData->AOTexture->Bind(0);
		LinearSamplerClamp().Bind(0);
		DrawScreenQuad();
		Texture::Unbind(1);
	}

}
