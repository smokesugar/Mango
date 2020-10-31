#pragma once

#include "Mango/Core/Base.h"
#include "Mango/Core/WinCore.h"
#include <d3d11.h>
#include <wrl.h>

#include "Mango/Renderer/SwapChain.h"
#include "DirectXFramebuffer.h"

namespace Mango {
	
	class DirectXSwapChain : public SwapChain {
	public:
		DirectXSwapChain();
		virtual void Present() override;
		virtual Ref<ColorBuffer> GetFramebuffer() override;
		virtual void Resize(uint32_t width, uint32_t height) override;
	private:
		void CreateInternalFramebuffer(uint32_t width, uint32_t height);
	private:
		Ref<DirectXColorBuffer> mFramebuffer;
		Microsoft::WRL::ComPtr<IDXGISwapChain> mInternal;
	};

}