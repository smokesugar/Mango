#pragma once

#include <d3d11.h>
#include <wrl.h>

#include "Mango/Renderer/Texture.h"

namespace Mango {
	
	class DirectXTexture2D : public Texture2D {
	public:
		DirectXTexture2D(void* data, uint32_t width, uint32_t height);
		DirectXTexture2D(const std::string& filePath, bool sRGB);

		inline virtual const std::string& GetPath() const override { return mPath; }

		inline virtual uint32_t GetWidth() const override { return mWidth; }
		inline virtual uint32_t GetHeight() const override { return mHeight; }

		virtual void Bind(size_t slot) const override;
	private:
		void Create(void* data, bool sRGB);
	private:
		std::string mPath;
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