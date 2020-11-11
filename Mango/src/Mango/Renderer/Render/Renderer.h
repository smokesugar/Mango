#pragma once

#include "Mango/Core/Base.h"
#include "Mango/Renderer/VertexArray.h"
#include "Mango/Renderer/Camera.h"
#include "Mango/Renderer/Texture.h"
#include "Mango/Renderer/Mesh.h"
#include "Mango/Renderer/RenderTarget.h"

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
		const Ref<Texture>& GetWhiteTexture();
		const Ref<Texture>& GetBlackTexture();
		Ref<Material> CreateDefaultMaterial();

		void BeginScene(const xmmatrix& projection, const xmmatrix& transform, uint32_t width, uint32_t height);
		void EndScene(const Ref<Texture>& target);

		void SubmitQuad(const xmmatrix& previousFrameTransform, const xmmatrix& transform, const Ref<Texture>& texture, const float4& color);
		void SubmitMesh(const Mesh& mesh, const xmmatrix& previousFrameTransform, const xmmatrix& transform);
	}

}