#include "ViewportInteraction.h"

namespace Mango {

	struct SolidColorShaderData {
		xmmatrix MVP;
		float3 Val;
		float padding;
	};

	struct MyData {
		Ref<Texture> RenderTarget;
		Ref<Mango::DepthBuffer> DepthBuffer;
		Ref<Texture> StagingTexture;
		Ref<Shader> SolidColorShader;
		Ref<Shader> OutlineShader;
		Ref<UniformBuffer> SolidColorUniforms;
		Ref<UniformBuffer> OutlineUniforms;
	};

	static MyData* sData;

	void ViewportInteraction::Init()
	{
		sData = new MyData();
		sData->RenderTarget = Ref<Texture>(Texture::Create(nullptr, 800, 600, Format::RGBA8_UNORM, Texture_RenderTarget));
		sData->DepthBuffer = Ref<DepthBuffer>(DepthBuffer::Create(800, 600));
		sData->StagingTexture = Ref<Texture>(Texture::Create(nullptr, 800, 600, Format::RGBA8_UNORM, Texture_CPU));
		sData->SolidColorShader = Ref<Shader>(Shader::Create("assets/shaders/SolidColor_vs.cso", "assets/shaders/SolidColor_ps.cso"));
		sData->OutlineShader = Ref<Shader>(Shader::Create("assets/shaders/Fullscreen_vs.cso", "assets/shaders/Outline_ps.cso"));
		sData->SolidColorUniforms = Ref<UniformBuffer>(UniformBuffer::Create<SolidColorShaderData>());
		sData->OutlineUniforms = Ref<UniformBuffer>(UniformBuffer::Create<float4>());
	}

	void ViewportInteraction::Shutdown()
	{
		delete sData;
	}

	const Ref<Texture> ViewportInteraction::GetRenderTarget()
	{
		return sData->RenderTarget;
	}

	struct RGB32 {
		union {
			struct { uint8_t r, g, b, a; };
			uint32_t raw;
		};

		RGB32()
			: r(0), g(0), b(0), a(1u)
		{}
	};

	static void RenderNode(const Node& node, const xmmatrix& parentTransform, const xmmatrix& viewProjection, float3 color) {
		xmmatrix transform = node.Transform * parentTransform;

		for (auto& submesh : node.Submeshes) {
			sData->SolidColorUniforms->SetData<SolidColorShaderData>({ transform * viewProjection, color });
			submesh.VA->Bind();
			if (submesh.VA->IsIndexed())
				RenderCommand::DrawIndexed(submesh.VA->GetDrawCount(), 0);
			else
				RenderCommand::Draw(submesh.VA->GetDrawCount(), 0);
		}

		for (auto& child : node.Children) {
			RenderNode(child, transform, viewProjection, color);
		}
	}

	ECS::Entity ViewportInteraction::GetHoveredEntity(const Ref<Scene>& scene, const float2& viewportSize, const float2& mousePosition, const xmmatrix& viewProjection)
	{
		sData->RenderTarget->EnsureSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
		sData->DepthBuffer->EnsureSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
		sData->StagingTexture->EnsureSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
		BindRenderTargets({ sData->RenderTarget }, sData->DepthBuffer);
		sData->RenderTarget->Clear(float4(0.0f, 0.0f, 0.0f, 1.0f));
		sData->DepthBuffer->Clear(0.0f);
		sData->SolidColorShader->Bind();
		sData->SolidColorUniforms->VSBind(0);

		std::unordered_map<uint32_t, ECS::Entity> valueEntityMap;
		RGB32 nullcolor;
		valueEntityMap[nullcolor.raw] = ECS::Null;
		float3 col = float3(0.0f, 0.0f, 0.0f);
		float increment = 1.0f / 255.0f;

		auto& reg = scene->GetRegistry();

		auto query0 = reg.QueryE<MeshComponent, TransformComponent>();
		for (auto& [size, entities, meshes, transforms] : query0) {
			for (size_t i = 0; i < size; i++) {
				auto& comp = meshes[i];
				auto& transform = transforms[i];

				col.x += increment;
				if (col.x > 1.0f) {
					col.y += increment;
					col.x = 0.0f;
				}
				if (col.y > 1.0f) {
					col.z += increment;
					col.y = 0.0f;
				}

				RGB32 colorint;
				colorint.r = (uint8_t)(col.x * 255.0f);
				colorint.g = (uint8_t)(col.y * 255.0f);
				colorint.b = (uint8_t)(col.z * 255.0f);
				colorint.a = 1u;
				valueEntityMap[colorint.raw] = entities[i];

				if (comp.MeshIndex != -1) {
					auto& mesh = scene->GetMeshLibrary()[comp.MeshIndex].second;
					RenderNode(mesh->RootNode, transform.GetMatrix(), viewProjection, col);
				}
			}
		}
		Renderer::GetSpriteQuadVertexArray()->Bind();
		auto query1 = reg.QueryE<SpriteRendererComponent, TransformComponent>();
		for (auto& [size, entities, sprites, transforms] : query1) {
			for (size_t i = 0; i < size; i++) {
				auto& transform = transforms[i];

				col.x += increment;
				if (col.x > 1.0f) {
					col.y += increment;
					col.x = 0.0f;
				}
				if (col.y > 1.0f) {
					col.z += increment;
					col.y = 0.0f;
				}

				RGB32 colorint;
				colorint.r = (uint8_t)(col.x * 255.0f);
				colorint.g = (uint8_t)(col.y * 255.0f);
				colorint.b = (uint8_t)(col.z * 255.0f);
				colorint.a = 1u;

				valueEntityMap[colorint.raw] = entities[i];

				sData->SolidColorUniforms->SetData<SolidColorShaderData>({ transform.GetMatrix() * viewProjection, col });
				RenderCommand::DrawIndexed(Renderer::GetSpriteQuadVertexArray()->GetDrawCount(), 0);
			}
		}

		uint8_t color[3];
		sData->RenderTarget->GetData((uint32_t)mousePosition.x, (uint32_t)mousePosition.y, sData->StagingTexture, color, sizeof(color));

		RGB32 colorint;
		memcpy(&colorint, color, sizeof(color));
		colorint.a = 1u;

		return valueEntityMap[colorint.raw];
	}

	void ViewportInteraction::RenderSelectionOutline(const Ref<Scene>& scene, ECS::Entity entity, const xmmatrix& viewProjection, const Ref<Texture>& rendertarget)
	{
		sData->RenderTarget->EnsureSize(rendertarget->GetWidth(), rendertarget->GetHeight());
		BindRenderTargets({ sData->RenderTarget });
		sData->RenderTarget->Clear(float4(0.0f, 0.0f, 0.0f, 1.0f));
		sData->SolidColorShader->Bind();
		sData->SolidColorUniforms->VSBind(0);

		auto& reg = scene->GetRegistry();

		if (reg.Has<MeshComponent>(entity)) {
			auto& comp = reg.Get<MeshComponent>(entity);
			auto& transform = reg.Get<TransformComponent>(entity);

			if (comp.MeshIndex != -1) {
				auto& mesh = scene->GetMeshLibrary()[comp.MeshIndex].second;
				RenderNode(mesh->RootNode, transform.GetMatrix(), viewProjection, float3(1.0f, 1.0f, 1.0f));
			}
		}

		if (reg.Has<SpriteRendererComponent>(entity)) {
			auto& sprite = reg.Get<SpriteRendererComponent>(entity);
			auto& transform = reg.Get<TransformComponent>(entity);
			
			sData->SolidColorUniforms->SetData<SolidColorShaderData>({ transform.GetMatrix() * viewProjection, float3(1.0f, 1.0f, 1.0f) });
			Renderer::GetSpriteQuadVertexArray()->Bind();
			RenderCommand::DrawIndexed(Renderer::GetSpriteQuadVertexArray()->GetDrawCount(), 0);
		}

		BindRenderTargets({ rendertarget });
		sData->OutlineShader->Bind();
		sData->RenderTarget->Bind(0);
		//sData->RenderTarget->Bind(0);
		sData->OutlineUniforms->PSBind(0);
		sData->OutlineUniforms->SetData(float4(1.0f, 0.6f, 0.0f, 1.0f));
		Renderer::LinearSamplerClamp().Bind(0);
		RenderCommand::EnableBlending();
		Renderer::DrawScreenQuad();
		RenderCommand::DisableBlending();
		Texture::Unbind(0); // Unbind so you can render with the texture
	}

}
