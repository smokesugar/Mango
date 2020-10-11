#include "mgpch.h"
#include "DirectXTexture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Mango/Core/Application.h"
#include "Mango/Core/Base.h"
#include "DirectXContext.h"

namespace Mango {

	Texture2D* Texture2D::Create(const std::string& filePath) {
		return new DirectXTexture2D(filePath);
	}

	DirectXTexture2D::DirectXTexture2D(const std::string& filePath)
		: mWidth(0), mHeight(0)
	{
		int width, height;
		unsigned char* data = stbi_load(filePath.c_str(), &width, &height, nullptr, 4);
		MG_CORE_ASSERT(data, "Could not load texture '{0}'.", filePath);
		mWidth = (uint32_t)width;
		mHeight = (uint32_t)height;
		Create(data);
		stbi_image_free(data);
	}

	void DirectXTexture2D::Bind(size_t slot) const
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->PSSetShaderResources((uint32_t)slot, 1, mSRV.GetAddressOf()));
	}

	void DirectXTexture2D::Create(unsigned char* data)
	{
		auto& context = RetrieveContext();

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = mWidth;
		desc.Height = mHeight;
		desc.MipLevels = 0;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc = {1, 0};
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		HR_CALL(context.GetDevice()->CreateTexture2D(&desc, nullptr, &texture));
		VOID_CALL(context.GetDeviceContext()->UpdateSubresource(texture.Get(), 0, nullptr, data, mWidth*4, 0));
		HR_CALL(context.GetDevice()->CreateShaderResourceView(texture.Get(), nullptr, &mSRV));
		VOID_CALL(context.GetDeviceContext()->GenerateMips(mSRV.Get()));
	}

	SamplerState* SamplerState::Create(Mode mode) {
		return new DirectXSamplerState(mode);
	}

	DirectXSamplerState::DirectXSamplerState(Mode mode)
	{
		auto& context = RetrieveContext();

		D3D11_SAMPLER_DESC desc = {};
		desc.Filter = GetFilter(mode);
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.MinLOD = 0;
		desc.MaxLOD = D3D11_FLOAT32_MAX;

		HR_CALL(context.GetDevice()->CreateSamplerState(&desc, &mInternal));
	}

	void DirectXSamplerState::Bind(size_t slot) const
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->PSSetSamplers((uint32_t)slot, 1, mInternal.GetAddressOf()));
	}

	D3D11_FILTER DirectXSamplerState::GetFilter(Mode mode)
	{
		D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		switch (mode) {
		case SamplerState::Mode::Linear:
			filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		case SamplerState::Mode::Point:
			filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			break;
		}
		return filter;
	}

}