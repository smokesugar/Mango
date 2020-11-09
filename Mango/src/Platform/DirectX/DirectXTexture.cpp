#include "mgpch.h"
#include "DirectXTexture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Mango/Core/Application.h"
#include "Mango/Core/Base.h"
#include "DirectXContext.h"

namespace Mango {

	Texture2D* Texture2D::Create(const std::string& filePath, Format format, bool mipMap) {
		return new DirectXTexture2D(filePath, format, mipMap);
	}

	Texture2D* Texture2D::Create(void* data, uint32_t width, uint32_t height) {
		return new DirectXTexture2D(data, width, height);
	}

	void Texture::Unbind(size_t slot) {
		auto& context = RetrieveContext();
		ID3D11ShaderResourceView* pp[] = { nullptr };
		VOID_CALL(context.GetDeviceContext()->PSSetShaderResources((uint32_t)slot, 1, pp));
	}

	DirectXTexture2D::DirectXTexture2D(void* data, uint32_t width, uint32_t height)
		: mWidth(width), mHeight(height)
	{
		Create(data, Format::RGBA8_UNORM, false);
	}

	DirectXTexture2D::DirectXTexture2D(const std::string& filePath, Format format, bool mipMap)
		: mWidth(0), mHeight(0), mPath(filePath)
	{
		int width, height;
		void* data;
		if (IsFormatFloatingPoint(format)) {
			data = stbi_loadf(filePath.c_str(), &width, &height, nullptr, 4);
			MG_CORE_INFO("Loading floating point image.");
		}
		else
			data = stbi_load(filePath.c_str(), &width, &height, nullptr, 4);

		MG_CORE_ASSERT(data, "Could not load texture '{0}'.", filePath);
		mWidth = (uint32_t)width;
		mHeight = (uint32_t)height;
		Create(data, format, mipMap);
		stbi_image_free(data);
	}

	void DirectXTexture2D::Bind(size_t slot) const
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->PSSetShaderResources((uint32_t)slot, 1, mSRV.GetAddressOf()));
	}

	void DirectXTexture2D::Create(void* data, Format format, bool mipMap)
	{
		auto& context = RetrieveContext();

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = mWidth;
		desc.Height = mHeight;
		desc.MipLevels = mipMap ? 0 : 1;
		desc.ArraySize = 1;
		desc.Format = DXGIFormatFromMangoFormat(format);
		desc.SampleDesc = {1, 0};
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		HR_CALL(context.GetDevice()->CreateTexture2D(&desc, nullptr, &texture));
		VOID_CALL(context.GetDeviceContext()->UpdateSubresource(texture.Get(), 0, nullptr, data, mWidth*FormatSize(format), 0));
		HR_CALL(context.GetDevice()->CreateShaderResourceView(texture.Get(), nullptr, &mSRV));
		if(mipMap)
			VOID_CALL(context.GetDeviceContext()->GenerateMips(mSRV.Get()));
	}

	SamplerState* SamplerState::Create(Filter mode, Address address, bool comparison) {
		return new DirectXSamplerState(mode, address, comparison);
	}

	DirectXSamplerState::DirectXSamplerState(Filter mode, Address address, bool comparison)
	{
		auto& context = RetrieveContext();

		D3D11_SAMPLER_DESC desc = {};
		desc.Filter = GetFilter(mode, comparison);
		desc.AddressU = GetAddress(address);
		desc.AddressV = GetAddress(address);
		desc.AddressW = GetAddress(address);
		desc.MinLOD = 0;
		desc.MaxLOD = D3D11_FLOAT32_MAX;
		desc.ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL;

		HR_CALL(context.GetDevice()->CreateSamplerState(&desc, &mInternal));
	}

	void DirectXSamplerState::Bind(size_t slot) const
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->PSSetSamplers((uint32_t)slot, 1, mInternal.GetAddressOf()));
	}

	D3D11_FILTER DirectXSamplerState::GetFilter(Filter mode, bool comparison)
	{
		D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		switch (mode) {
		case SamplerState::Filter::Linear:
			filter = comparison ? D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		case SamplerState::Filter::Point:
			filter = comparison ? D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR : D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			break;
		}
		return filter;
	}

	D3D11_TEXTURE_ADDRESS_MODE DirectXSamplerState::GetAddress(Address mode)
	{
		D3D11_TEXTURE_ADDRESS_MODE add = D3D11_TEXTURE_ADDRESS_WRAP;
		switch (mode) {
		case SamplerState::Address::Wrap:
			add = D3D11_TEXTURE_ADDRESS_WRAP;
			break;
		case SamplerState::Address::Clamp:
			add = D3D11_TEXTURE_ADDRESS_CLAMP;
			break;
		case SamplerState::Address::Border:
			add = D3D11_TEXTURE_ADDRESS_BORDER;
			break;
		}
		return add;
	}

	Cubemap* Cubemap::Create(const std::string& path, uint32_t size) {
		return new DirectXCubemap(path, size);
	}

	DirectXCubemap::DirectXCubemap(const std::string& path, uint32_t size)
		: mPath(path), mWidth(size), mHeight(size)
	{
		auto& context = RetrieveContext();

		DXGI_FORMAT format = DXGI_FORMAT_R16G16B16A16_FLOAT;

		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = mWidth;
		texDesc.Height = mHeight;
		texDesc.MipLevels = 0;
		texDesc.ArraySize = 6;
		texDesc.Format = format;
		texDesc.CPUAccessFlags = 0;
		texDesc.SampleDesc = { 1, 0 };
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = -1;
		srvDesc.TextureCube.MostDetailedMip = 0;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		HR_CALL(context.GetDevice()->CreateTexture2D(&texDesc, nullptr, &texture));
		HR_CALL(context.GetDevice()->CreateShaderResourceView(texture.Get(), &srvDesc, &mSRV));

		D3D11_TEXTURE2D_DESC retrievedDesc;
		texture->GetDesc(&retrievedDesc);
		mMipLevels = retrievedDesc.MipLevels;
		mRTVs.resize(mMipLevels);

		for (int i = 0; i < mMipLevels; i++) {
			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = format;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Texture2DArray.MipSlice = i;
			rtvDesc.Texture2DArray.FirstArraySlice = 0;
			rtvDesc.Texture2DArray.ArraySize = 6;

			HR_CALL(context.GetDevice()->CreateRenderTargetView(texture.Get(), &rtvDesc, &mRTVs[i]));
		}
	}

	void DirectXCubemap::GenerateMips()
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->GenerateMips(mSRV.Get()));
	}

	uint32_t DirectXCubemap::GetMipLevels() const
	{
		return mMipLevels;
	}

	void DirectXCubemap::BindAsRenderTarget(size_t mip) const
	{
		MG_CORE_ASSERT(mip < mMipLevels, "Index out of bounds.");

		auto& context = RetrieveContext();

		D3D11_VIEWPORT vp;
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.Width  = (float)mWidth / (powf(2.0f, (float)mip));
		vp.Height = (float)mHeight / (powf(2.0f, (float)mip));
		VOID_CALL(context.GetDeviceContext()->RSSetViewports(1, &vp));

		VOID_CALL(context.GetDeviceContext()->OMSetRenderTargets(1, mRTVs[mip].GetAddressOf(), nullptr));
	}

	void DirectXCubemap::Bind(size_t slot) const
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->PSSetShaderResources((uint32_t)slot, 1, mSRV.GetAddressOf()));
	}
}
