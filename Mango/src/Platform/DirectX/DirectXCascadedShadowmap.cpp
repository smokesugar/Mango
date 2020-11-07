#include "mgpch.h"
#include "DirectXCascadedShadowmap.h"

#include "Mango/Core/Application.h"
#include "DirectXContext.h"

namespace Mango {
	
	CascadedShadowmap* CascadedShadowmap::Create(uint32_t width, uint32_t height, uint32_t cascades) {
		return new DirectXCascadedShadowmap(width, height, cascades);
	}

	DirectXCascadedShadowmap::DirectXCascadedShadowmap(uint32_t width, uint32_t height, uint32_t cascades)
		: mWidth(width), mHeight(height)
	{
		auto& context = RetrieveContext();

		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = cascades;
		textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		textureDesc.SampleDesc = { 1, 0 };
		textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		dsvDesc.Texture2DArray.MipSlice = 0;
		dsvDesc.Texture2DArray.FirstArraySlice = 0;
		dsvDesc.Texture2DArray.ArraySize = cascades;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.MipLevels = 1;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.ArraySize = cascades;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		HR_CALL(context.GetDevice()->CreateTexture2D(&textureDesc, nullptr, &texture));
		HR_CALL(context.GetDevice()->CreateDepthStencilView(texture.Get(), &dsvDesc, &mDSV));
		HR_CALL(context.GetDevice()->CreateShaderResourceView(texture.Get(), &srvDesc, &mSRV));
	}

	void DirectXCascadedShadowmap::Clear(float value)
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->ClearDepthStencilView(mDSV.Get(), D3D11_CLEAR_DEPTH, value, 0));
	}

	void DirectXCascadedShadowmap::BindAsRenderTarget() const
	{
		auto& context = RetrieveContext();

		D3D11_VIEWPORT vp;
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.Width = (float)mWidth;
		vp.Height = (float)mHeight;

		VOID_CALL(context.GetDeviceContext()->RSSetViewports(1, &vp));
		VOID_CALL(context.GetDeviceContext()->OMSetRenderTargets(0, nullptr, mDSV.Get()));
	}

	void DirectXCascadedShadowmap::BindAsTexture(size_t slot) const
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->PSSetShaderResources((uint32_t)slot, 1, mSRV.GetAddressOf()));
	}

}
