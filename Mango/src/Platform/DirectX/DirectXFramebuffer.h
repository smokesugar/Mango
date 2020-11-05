#pragma once

#include <d3d11.h>
#include <wrl.h>

#include "Mango/Renderer/Framebuffer.h"

namespace Mango {
	
	class DirectXColorBuffer : public ColorBuffer {
		friend class ColorBuffer;
	public:
		DirectXColorBuffer(const ColorBufferProperties& props);
		DirectXColorBuffer(ID3D11Resource* resource, uint32_t width, uint32_t height);

		virtual void Clear(const float4& color) override;
		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual void EnsureSize(uint32_t width, uint32_t height) override;

		virtual uint32_t GetWidth() const override { return mProps.Width; }
		virtual uint32_t GetHeight() const override { return mProps.Height; }

		virtual void* GetTextureAttachment() const override { return mSRV.Get(); }
		virtual void BindAsTexture(size_t slot) const override;

		inline ID3D11RenderTargetView* GetRenderTargetView() { return mDSV.Get(); }
	private:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> CreateTexture();
		void CreateViews(ID3D11Resource* resource);
	private:
		bool mOwnsTexture;
		ColorBufferProperties mProps;
		ID3D11Resource* mResourceReference;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mDSV;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSRV;
	};

	class DirectXDepthBuffer : public DepthBuffer {
	public:
		DirectXDepthBuffer(uint32_t width, uint32_t height);
		virtual void Clear(float value) override;
		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual void EnsureSize(uint32_t width, uint32_t height) override;
		virtual void BindAsTexture(size_t slot) const override;

		virtual uint32_t GetWidth() const override { return mWidth; }
		virtual uint32_t GetHeight() const override { return mHeight; }

		inline ID3D11DepthStencilView* GetDepthStencilView() { return mDSV.Get(); }
	private:
		void CreateViews(uint32_t width, uint32_t height);
	private:
		uint32_t mWidth, mHeight;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDSV;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSRV;
	};

}