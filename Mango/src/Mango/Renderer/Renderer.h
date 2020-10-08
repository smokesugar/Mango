#pragma once

#include "Mango/Core/Base.h"
#include "VertexArray.h"
#include "Camera.h"

namespace Mango {

	class Renderer {
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(Camera& camera, const xmmatrix& transform);
		static void EndScene();

		static void Submit(const Ref<VertexArray>& va, const xmmatrix& transform = DirectX::XMMatrixIdentity());
	};

}