#pragma once

#include "Mango/Core/Base.h"
#include "Mango/Core/WinCore.h"
#include <d3d11.h>
#include <wrl.h>

#include "Mango/Renderer/SwapChain.h"
#include "DirectXRenderTarget.h"

namespace Mango {
	
	class DirectXSwapChain : public SwapChain {
	public:
		DirectXSwapChain();
		virtual void Present() override;
		virtual void BindAsRenderTarget() override;
		virtual void Resize(uint32_t width, uint32_t height) override;
	private:
		void CreateRenderTargetView(uint32_t width, uint32_t height);
	private:
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mRTV;
		Microsoft::WRL::ComPtr<IDXGISwapChain> mInternal;
	};

}