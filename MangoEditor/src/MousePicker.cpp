#include "MousePicker.h"

namespace Mango {

	struct MousePickerData {
		xmmatrix MVP;
		float3 Val;
		float padding;
	};
	
	struct MyData {
		Ref<Texture> MousePickerTexture;
		Ref<DepthBuffer> MousePickerDepthBuffer;
		Ref<Texture> MousePickerTextureStaging;
		Ref<Shader> MousePickerShader;
		Ref<UniformBuffer> MousePickerUniforms;
	};

	static MyData* sData;

	void MousePicker::Init()
	{
		sData = new MyData();
		sData->MousePickerTexture = Ref<Texture>(Texture::Create(nullptr, 800, 600, Format::RGBA8_UNORM, Texture_RenderTarget));
		sData->MousePickerDepthBuffer = Ref<DepthBuffer>(DepthBuffer::Create(800, 600));
		sData->MousePickerTextureStaging = Ref<Texture>(Texture::Create(nullptr, 800, 600, Format::RGBA8_UNORM, Texture_CPU));
		sData->MousePickerShader = Ref<Shader>(Shader::Create("assets/shaders/MousePicker_vs.cso", "assets/shaders/MousePicker_ps.cso"));
		sData->MousePickerUniforms = Ref<UniformBuffer>(UniformBuffer::Create<MousePickerData>());
	}

	void MousePicker::Shutdown()
	{
		delete sData;
	}

	struct RGB32 {
		union {
			struct { uint8_t r, g, b; };
			uint32_t raw;
		};

		RGB32()
			: r(0), g(0), b(0)
		{}
	};

	static void RenderNode(const Node& node, const xmmatrix& parentTransform, const xmmatrix& viewProjection, float3 color) {
		xmmatrix transform = node.Transform * parentTransform;

		for (auto& [va, mat] : node.Submeshes) {
			sData->MousePickerUniforms->SetData<MousePickerData>({transform * viewProjection, color });
			va->Bind();
			if (va->IsIndexed())
				RenderCommand::DrawIndexed(va->GetDrawCount(), 0);
			else
				RenderCommand::Draw(va->GetDrawCount(), 0);
		}

		for (auto& child : node.Children) {
			RenderNode(child, transform, viewProjection, color);
		}
	}

	ECS::Entity MousePicker::GetHoveredEntity(ECS::Registry& reg, const float2& viewportSize, const float2& mousePosition, const xmmatrix& viewProjection)
	{
		sData->MousePickerTexture->EnsureSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
		sData->MousePickerDepthBuffer->EnsureSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
		sData->MousePickerTextureStaging->EnsureSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
		BindRenderTargets({ sData->MousePickerTexture }, sData->MousePickerDepthBuffer);
		sData->MousePickerTexture->Clear(float4(0.0f, 0.0f, 0.0f, 1.0f));
		sData->MousePickerDepthBuffer->Clear(0.0f);
		sData->MousePickerShader->Bind();
		sData->MousePickerUniforms->VSBind(0);

		std::unordered_map<uint32_t, ECS::Entity> valueEntityMap;
		RGB32 nullcolor;
		valueEntityMap[nullcolor.raw] = ECS::Null;
		float3 col = float3(0.0f, 0.0f, 0.0f);
		float increment = 1.0f / 255.0f;

		auto query0 = reg.QueryE<MeshComponent, TransformComponent>();
		for (auto& [size, entities, meshes, transforms] : query0) {
			for (size_t i = 0; i < size; i++) {
				auto& mesh = meshes[i];
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

				valueEntityMap[colorint.raw] = entities[i];
				RenderNode(mesh.Mesh.RootNode, transform.GetMatrix(), viewProjection, col);
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

				valueEntityMap[colorint.raw] = entities[i];

				sData->MousePickerUniforms->SetData<MousePickerData>({transform.GetMatrix() * viewProjection, col });
				RenderCommand::DrawIndexed(Renderer::GetSpriteQuadVertexArray()->GetDrawCount(), 0);
			}
		}

		uint8_t color[3];
		sData->MousePickerTexture->GetData((uint32_t)mousePosition.x, (uint32_t)mousePosition.y, sData->MousePickerTextureStaging, color, sizeof(color));

		RGB32 colorint;
		memcpy(&colorint, color, sizeof(color));

		return valueEntityMap[colorint.raw];
	}

}
