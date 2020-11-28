#pragma once

#include "Mango.h"

namespace Mango {
	
	class RuntimeLayer : public Layer {
	public:
		RuntimeLayer();
		~RuntimeLayer();

		virtual void OnUpdate(float dt) override;
	private:
		Ref<Scene> mScene;
		Ref<Texture> mRenderTarget;
	};

}