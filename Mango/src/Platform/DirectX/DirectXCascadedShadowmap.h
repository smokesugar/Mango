#pragma once

#include <wrl.h>
#include <d3d11.h>

#include "Mango/Renderer/CascadedShadowmap.h"

namespace Mango {
	
	class DirectXCascadedShadowmap : public CascadedShadowmap {
	public:
		DirectXCascadedShadowmap(uint32_t width, uint32_t height, uint32_t cascades);

		virtual void Clear(float value) override;

		virtual void BindAsRenderTarget() const override;
		virtual void BindAsTexture(size_t slot) const override;
	private:
		uint32_t mWidth, mHeight;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSRV;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDSV;
	};

}