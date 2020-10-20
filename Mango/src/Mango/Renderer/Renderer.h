#pragma once

#include "Mango/Core/Base.h"
#include "VertexArray.h"
#include "Camera.h"
#include "Texture.h"
#include "Mesh.h"

namespace Mango {

	namespace Renderer {
		void Init();
		void Shutdown();

		bool& TAAEnabled();

		void BeginScene(const xmmatrix& projection, const xmmatrix& transform, uint32_t width, uint32_t height);
		void EndScene();

		void DrawQuad(const float3& pos, const float2& size, const float4& color);
		void DrawQuad(const float3& pos, const float2& size, const Ref<Texture2D>& texture);
		void DrawQuad(const xmmatrix& transform, const float4& color);
		void DrawQuad(const xmmatrix& transform, const Ref<Texture2D>& texture);

		void DrawScreenQuad();

		void SubmitMesh(const Mesh& mesh, const xmmatrix& transform);
	}

}