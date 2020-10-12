#pragma once

#include "Mango/Core/Base.h"
#include "VertexArray.h"
#include "Camera.h"
#include "Texture.h"

namespace Mango {

	namespace Renderer {
		void Init();
		void Shutdown();

		void BeginScene(Camera& camera, const xmmatrix& transform);
		void EndScene();

		void DrawQuad(const float3& pos, const float2& size, const float4& color);
		void DrawQuad(const float3& pos, const float2& size, const Ref<Texture2D>& texture);
	}

}