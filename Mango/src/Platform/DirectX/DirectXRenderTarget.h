#pragma once

#include <d3d11.h>
#include <wrl.h>

#include "Mango/Renderer/RenderTarget.h"

namespace Mango {

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