#pragma once

#include <d3d11.h>
#include <wrl.h>

#include "Mango/Renderer/Framebuffer.h"

namespace Mango {
	
	class DirectXFramebuffer : public Framebuffer {
		friend class Framebuffer;
	public:
		DirectXFramebuffer(const FramebufferProperties& props);
		DirectXFramebuffer(ID3D11Resource* resource, uint32_t width, uint32_t height, bool depth);

		virtual void Bind() override;
		virtual void Clear(const float4& color) override;
		virtual void ClearDepth() override;
		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual void EnsureSize(uint32_t width, uint32_t height) override;

		virtual uint32_t GetWidth() const override { return mProps.Width; }
		virtual uint32_t GetHeight() const override { return mProps.Height; }

		virtual void* GetTextureAttachment() const override { return mSRV.Get(); }
		virtual void BindAsTexture(size_t slot) const override;
		virtual void BindDepthAsTexture(size_t slot) const override;

		inline ID3D11RenderTargetView* GetRenderTargetView() { return mRTV.Get(); }
	private:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> CreateTexture();
		void CreateViews(ID3D11Resource* resource);
	private:
		bool mOwnsTexture;
		FramebufferProperties mProps;
		ID3D11Resource* mResourceReference;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mRTV;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSRV;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDSV;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mDSRV;
	};

}