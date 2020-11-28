#include "mgpch.h"
#include "DirectXRenderTarget.h"

#include "Mango/Core/Application.h"
#include "DirectXContext.h"
#include "DirectXTexture.h"
#include "DirectXSwapChain.h"

namespace Mango {

	void BindRenderTargets(const std::vector<Ref<Texture>>& framebuffers, const Ref<DepthBuffer>& depthbuffer) {
		if (framebuffers.empty() && !depthbuffer)
			return;

		std::vector<ID3D11RenderTargetView*> rtvs;

		for (auto& framebuffer : framebuffers) {
			auto dxframebuffer = std::static_pointer_cast<DirectXTexture>(framebuffer);
			rtvs.push_back(dxframebuffer->GetRenderTargetView());
		}

		auto& context = RetrieveContext();

		uint32_t width = framebuffers.empty() ? depthbuffer->GetWidth() : framebuffers[0]->GetWidth();
		uint32_t height = framebuffers.empty() ? depthbuffer->GetHeight() : framebuffers[0]->GetHeight();

		D3D11_VIEWPORT vp;
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.Width = (float)width;
		vp.Height = (float)height;

		auto dsv = depthbuffer ? std::static_pointer_cast<DirectXDepthBuffer>(depthbuffer)->GetDepthStencilView() : nullptr;

		VOID_CALL(context.GetDeviceContext()->RSSetViewports(1, &vp));
		VOID_CALL(context.GetDeviceContext()->OMSetRenderTargets((uint32_t)rtvs.size(), rtvs.data(), dsv));
	}

	void BlitToSwapChain(SwapChain& dest, const Ref<Texture>& src)
	{
		auto destX = (DirectXSwapChain*)(&dest);
		auto srcX = std::static_pointer_cast<DirectXTexture>(src);
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->CopyResource(destX->GetBackBuffer(), srcX->GetInternalTexture()));
	}

	// DEPTH BUFFER ----------------------------------------------------------------------------------

	DepthBuffer* DepthBuffer::Create(uint32_t width, uint32_t height) {
		return new DirectXDepthBuffer(width, height);
	}

	DirectXDepthBuffer::DirectXDepthBuffer(uint32_t width, uint32_t height)
	{
		CreateViews(width, height);
	}

	void DirectXDepthBuffer::Clear(float value)
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->ClearDepthStencilView(mDSV.Get(), D3D11_CLEAR_DEPTH, value, 0));
	}

	void DirectXDepthBuffer::Resize(uint32_t width, uint32_t height)
	{
		CreateViews(width, height);
	}

	void DirectXDepthBuffer::EnsureSize(uint32_t width, uint32_t height)
	{
		if (mWidth != width || mHeight != height)
			Resize(width, height);
	}

	void DirectXDepthBuffer::BindAsTexture(size_t slot) const
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->PSSetShaderResources((uint32_t)slot, 1, mSRV.GetAddressOf()));
	}

	void DirectXDepthBuffer::CreateViews(uint32_t width, uint32_t height)
	{
		mWidth = width;
		mHeight = height;

		auto& context = RetrieveContext();

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R32_TYPELESS;
		desc.SampleDesc = { 1, 0 };
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

		D3D11_SHADER_RESOURCE_VIEW_DESC sr_desc = {};
		sr_desc.Format = DXGI_FORMAT_R32_FLOAT;
		sr_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		sr_desc.Texture2D.MostDetailedMip = 0;
		sr_desc.Texture2D.MipLevels = -1;

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = 0;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthTexture;
		HR_CALL(context.GetDevice()->CreateTexture2D(&desc, nullptr, &depthTexture));
		HR_CALL(context.GetDevice()->CreateShaderResourceView(depthTexture.Get(), &sr_desc, &mSRV));
		HR_CALL(context.GetDevice()->CreateDepthStencilView(depthTexture.Get(), &dsvDesc, &mDSV));
	}

}