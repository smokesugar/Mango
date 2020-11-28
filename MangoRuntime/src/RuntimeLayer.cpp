#include "RuntimeLayer.h"

namespace Mango {

	RuntimeLayer::RuntimeLayer()
	{
		mScene = Serializer::DeserializeScene("RuntimeScene.json");
		
		mRenderTarget = Ref<Texture>(Texture::Create(nullptr, 800, 600, Format::RGBA8_UNORM, Texture_RenderTarget));

		mScene->Start();
	}

	RuntimeLayer::~RuntimeLayer()
	{
		mScene->Stop();
	}

	void RuntimeLayer::OnUpdate(float dt)
	{
		auto& window = Application::Get().GetWindow();
		mRenderTarget->EnsureSize(window.GetWidth(), window.GetHeight());
		mRenderTarget->Clear(RENDERER_CLEAR_COLOR);
		mScene->OnUpdate(dt, mRenderTarget);
		
		BlitToSwapChain(window.GetSwapChain(), mRenderTarget);
	}

}
