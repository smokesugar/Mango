#pragma once

#include <d3d11.h>
#include <wrl.h>

#include "Mango/Renderer/Framebuffer.h"

namespace Mango {
	
	class DirectXFramebuffer : public Framebuffer {
	public:
		DirectXFramebuffer(const FramebufferProperties& props);
		DirectXFramebuffer(ID3D11Resource* resource, uint32_t width, uint32_t height);
		virtual void Bind() override;
		virtual void Clear(const float4& color) override;
		virtual void Resize(uint32_t width, uint32_t height) override;
		inline ID3D11RenderTargetView* GetRenderTargetView() { return mRTV.Get(); }
	private:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> CreateTexture();
		void CreateRenderTargetView(ID3D11Resource* resource);
	private:
		bool mOwnsTexture;
		FramebufferProperties mProps;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mRTV;
	};

}