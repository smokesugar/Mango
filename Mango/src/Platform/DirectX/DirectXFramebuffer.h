#pragma once

#include <d3d11.h>
#include <wrl.h>

#include "Mango/Renderer/Framebuffer.h"

namespace Mango {
	
	class DirectXFramebuffer : public Framebuffer {
		friend class Framebuffer;
	public:
		DirectXFramebuffer(const FramebufferProperties& props);
		DirectXFramebuffer(ID3D11Resource* resource, uint32_t width, uint32_t height);

		virtual void Bind() override;
		virtual void Clear(const float4& color) override;
		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual void EnsureSize(uint32_t width, uint32_t height) override;

		virtual uint32_t GetWidth() const override { return mProps.Width; }
		virtual uint32_t GetHeight() const override { return mProps.Height; }

		inline ID3D11RenderTargetView* GetRenderTargetView() { return mRTV.Get(); }
	private:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> CreateTexture();
		void CreateRenderTargetView(ID3D11Resource* resource);
	private:
		bool mOwnsTexture;
		FramebufferProperties mProps;
		ID3D11Resource* mResourceReference;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mRTV;
	};

}