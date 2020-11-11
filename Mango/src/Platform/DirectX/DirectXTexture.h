#pragma once

#include <d3d11.h>
#include <wrl.h>

#include "Mango/Renderer/Texture.h"

namespace Mango {
	
	class DirectXTexture : public Texture {
	public:
		DirectXTexture(void* data, uint32_t width, uint32_t height, Format format, TextureFlags flags);
		DirectXTexture(const std::string& filePath, Format format, TextureFlags flags);

		inline virtual const std::string& GetPath() const override { return mPath; }
		inline virtual uint32_t GetWidth() const override { return mWidth; }
		inline virtual uint32_t GetHeight() const override { return mHeight; }
		inline virtual void* GetNativeTexture() const override { return mSRV.Get(); }

		virtual void EnsureSize(uint32_t width, uint32_t height) override;
		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual void Clear(float4 color) override;

		virtual void Bind(size_t slot) const override;

		inline ID3D11RenderTargetView* GetRenderTargetView() const { return mRTV.Get(); }
		inline ID3D11Texture2D* GetInternalTexture() const { return mTexture.Get(); }
	private:
		void Create(void* data);
	private:
		TextureFlags mFlags;
		std::string mPath;
		uint32_t mWidth, mHeight;
		Format mFormat;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> mTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSRV;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mRTV;
	};

	class DirectXCubemap : public Cubemap {
	public:
		DirectXCubemap(const std::string& path, uint32_t size);

		inline virtual const std::string& GetPath() const override { return mPath; }

		inline virtual uint32_t GetWidth() const override { return mWidth; }
		inline virtual uint32_t GetHeight() const override { return mHeight; }

		virtual void GenerateMips() override;
		virtual uint32_t GetMipLevels() const override;

		virtual void BindAsRenderTarget(size_t mip) const override;
		virtual void BindAsShaderResource(size_t slot) const override;
	private:
		uint32_t mMipLevels;
		std::string mPath;
		uint32_t mWidth, mHeight;
		std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> mRTVs;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSRV;
	};

	class DirectXSamplerState : public SamplerState {
	public:
		DirectXSamplerState(Filter mode, Address address, bool comparison);

		virtual void Bind(size_t slot) const override;
	private:
		static D3D11_FILTER GetFilter(Filter mode, bool comparison);
		static D3D11_TEXTURE_ADDRESS_MODE GetAddress(Address mode);
	private:
		Microsoft::WRL::ComPtr<ID3D11SamplerState> mInternal;
	};

}