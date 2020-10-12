#pragma once

#include "Mango.h"

using namespace Mango;

class SandboxLayer : public Layer {
public:
	SandboxLayer();

	void OnUpdate(float dt);
	void OnImGuiRender();
private:
	Ref<Scene> mScene;

	OrthographicCamera mCamera;
	Ref<Framebuffer> mFramebuffer;
	Ref<Texture2D> mTexture;

	float4 mSquareColor = float4(1.0f, 0.2f, 0.3f, 1.0f);

	float2 mViewportSize = { 800.0f, 600.0f };
};