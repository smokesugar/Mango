#pragma once

#include "Mango.h"

using namespace Mango;

class SandboxLayer : public Layer {
public:
	SandboxLayer();

	void OnUpdate(float dt);
private:
	Ref<Scene> mScene;

	OrthographicCamera mCamera;
	Ref<Framebuffer> mFramebuffer;
	Ref<Texture2D> mTexture;
};