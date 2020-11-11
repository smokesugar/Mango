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
	Ref<ColorBuffer> mFramebuffer;
	Ref<Texture> mTexture;
};