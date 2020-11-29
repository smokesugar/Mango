#pragma once

#include "Mango.h"

namespace Mango {
	
	class RuntimeLayer : public Layer {
	public:
		RuntimeLayer();
		~RuntimeLayer();

		virtual void OnUpdate(float dt) override;
		virtual void OnImGuiRender() override;
	private:
		Ref<Scene> mScene;
		Ref<Texture> mRenderTarget;
		float mFPS = 0.0f;
	};

}