#pragma once

#include "Mango.h"

using namespace Mango;

class SandboxLayer : public Layer {
public:
	SandboxLayer();

	void OnUpdate(float dt);
	void OnImGuiRender();
private:
	Ref<Shader> mShader;
	Ref<VertexArray> mQuad;
	OrthographicCamera mCamera;
	Ref<Framebuffer> mFramebuffer;
	Ref<Texture2D> mTexture;
	Ref<SamplerState> mSampler;
};