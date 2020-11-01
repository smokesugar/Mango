#pragma once

#include "Mango/Core/Base.h"
#include "VertexArray.h"
#include "Camera.h"
#include "Texture.h"
#include "Mesh.h"
#include "Framebuffer.h"

namespace Mango {

	static constexpr float4 RENDERER_CLEAR_COLOR = float4(0.0f, 0.0f, 0.0f, 1.0f);

	struct PreviousFrameTransformComponent {
		xmmatrix Transform;

		PreviousFrameTransformComponent(const PreviousFrameTransformComponent&) = default;
		PreviousFrameTransformComponent(const xmmatrix& transform = XMMatrixIdentity())
			: Transform(transform) {}
	};

	namespace Renderer {
		void Init();
		void Shutdown();

		bool& TAAEnabled();
		const Ref<Texture2D>& GetWhiteTexture();
		const Ref<Texture2D>& GetBlackTexture();
		Ref<Material> CreateDefaultMaterial();

		void BeginScene(const xmmatrix& projection, const xmmatrix& transform, uint32_t width, uint32_t height);
		void EndScene(const Ref<ColorBuffer>& target);

		void DrawQuad(const xmmatrix& previousFrameTransform, const xmmatrix& transform, const Ref<Texture2D>& texture, const float4& color);

		void DrawScreenQuad();

		void SubmitMesh(const Mesh& mesh, const xmmatrix& previousFrameTransform, const xmmatrix& transform);
	}

}