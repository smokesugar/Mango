#include "mgpch.h"
#include "DirectXFramebuffer.h"

#include "Mango/Core/Application.h"
#include "DirectXContext.h"

namespace Mango {

	ColorBuffer* ColorBuffer::Create(const ColorBufferProperties& props) {
		return new DirectXColorBuffer(props);
	}

	void ColorBuffer::Blit(const Ref<ColorBuffer>& dst, const Ref<ColorBuffer>& src) {
		MG_CORE_ASSERT(dst->GetWidth() == src->GetWidth() && dst->GetHeight() == src->GetHeight(), "Framebuffers must have identical dimensions to blit");
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->CopyResource(
			std::static_pointer_cast<DirectXColorBuffer>(dst)->mResourceReference,
			std::static_pointer_cast<DirectXColorBuffer>(src)->mResourceReference
		));
	}

	DirectXColorBuffer::DirectXColorBuffer(const ColorBufferProperties& props)
		: mProps(props), mOwnsTexture(true)
	{
		auto texture = CreateTexture();
		CreateViews(texture.Get());
	}

	DirectXColorBuffer::DirectXColorBuffer(ID3D11Resource* resource, uint32_t width, uint32_t height)
		: mProps({width, height}), mOwnsTexture(false)
	{
		CreateViews(resource);
	}

	void BindRenderTargets(const std::vector<Ref<ColorBuffer>>& framebuffers, const Ref<DepthBuffer>& depthbuffer) {
		if (framebuffers.empty() && !depthbuffer)
			return;

		std::vector<ID3D11RenderTargetView*> rtvs;

		for (auto& framebuffer : framebuffers) {
			auto dxframebuffer = std::static_pointer_cast<DirectXColorBuffer>(framebuffer);
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

	void DirectXColorBuffer::Clear(const float4& color)
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->ClearRenderTargetView(mDSV.Get(), ValuePtr(color)));
	}

	void DirectXColorBuffer::Resize(uint32_t width, uint32_t height)
	{
		MG_CORE_ASSERT(mOwnsTexture, "Cannot resize a framebuffer if it does not own its texture.");

		mProps.Width = width;
		mProps.Height = height;

		auto texture = CreateTexture();
		CreateViews(texture.Get());
	}

	void DirectXColorBuffer::EnsureSize(uint32_t width, uint32_t height)
	{
		if (mProps.Width != width || mProps.Height != height)
			Resize(width, height);
	}

	void DirectXColorBuffer::BindAsTexture(size_t slot) const
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->PSSetShaderResources((uint32_t)slot, 1, mSRV.GetAddressOf()));
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> DirectXColorBuffer::CreateTexture()
	{
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = mProps.Width;
		desc.Height = mProps.Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGIFormatFromMangoFormat(mProps.Format);
		desc.SampleDesc = {1, 0};
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		auto& context = RetrieveContext();
		HR_CALL(context.GetDevice()->CreateTexture2D(&desc, nullptr, &texture));
		return texture;
	}

	void DirectXColorBuffer::CreateViews(ID3D11Resource* resource)
	{
		mResourceReference = resource;
		auto& context = RetrieveContext();
		HR_CALL(context.GetDevice()->CreateRenderTargetView(resource, nullptr, &mDSV));

		if(mOwnsTexture)
			HR_CALL(context.GetDevice()->CreateShaderResourceView(resource, nullptr, &mSRV));
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