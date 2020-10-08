#pragma once

#include "Mango.h"

using namespace Mango;

class SandboxLayer : public Layer {
public:
	SandboxLayer();

	void OnUpdate(float dt);
private:
	Ref<Shader> mShader;
	Ref<VertexArray> mQuad;
	OrthographicCamera mCamera;

	Ref<UniformBuffer> mUniformBuffer;
};