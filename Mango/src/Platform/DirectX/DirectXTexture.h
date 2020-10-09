#pragma once

#include <d3d11.h>
#include <wrl.h>

#include "Mango/Renderer/Texture.h"

namespace Mango {
	
	class DirectXTexture2D : public Texture2D {
	public:
		DirectXTexture2D(const std::string& filePath);

		inline virtual uint32_t GetWidth() const override { return mWidth; }
		inline virtual uint32_t GetHeight() const override { return mHeight; }

		virtual void Bind(size_t slot) const override;
	private:
		void Create(unsigned char* data);
	private:
		uint32_t mWidth, mHeight;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSRV;
	};

	class DirectXSamplerState : public SamplerState {
	public:
		DirectXSamplerState(Mode mode);

		virtual void Bind(size_t slot) const override;
	private:
		static D3D11_FILTER GetFilter(Mode mode);
	private:
		Microsoft::WRL::ComPtr<ID3D11SamplerState> mInternal;
	};

}