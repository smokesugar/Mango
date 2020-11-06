#pragma once

#include "Mango/Core/Base.h"
#include "Mango/Renderer/VertexArray.h"
#include "Mango/Renderer/Camera.h"
#include "Mango/Renderer/Texture.h"
#include "Mango/Renderer/Mesh.h"
#include "Mango/Renderer/Framebuffer.h"

#include "RendererBase.h"
#include "RendererLighting.h"

namespace Mango {

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

		void SubmitQuad(const xmmatrix& previousFrameTransform, const xmmatrix& transform, const Ref<Texture2D>& texture, const float4& color);
		void SubmitMesh(const Mesh& mesh, const xmmatrix& previousFrameTransform, const xmmatrix& transform);
	}

}