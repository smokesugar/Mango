#include "mgpch.h"
#include "DirectXFramebuffer.h"

#include "Mango/Core/Application.h"
#include "DirectXContext.h"

namespace Mango {

	Framebuffer* Framebuffer::Create(const FramebufferProperties& props) {
		return new DirectXFramebuffer(props);
	}

	void Framebuffer::Blit(const Ref<Framebuffer>& dst, const Ref<Framebuffer>& src) {
		MG_CORE_ASSERT(dst->GetWidth() == src->GetWidth() && dst->GetHeight() == src->GetHeight(), "Framebuffers must have identical dimensions to blit");
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->CopyResource(
			std::static_pointer_cast<DirectXFramebuffer>(dst)->mResourceReference,
			std::static_pointer_cast<DirectXFramebuffer>(src)->mResourceReference
		));
	}

	DirectXFramebuffer::DirectXFramebuffer(const FramebufferProperties& props)
		: mProps(props), mOwnsTexture(true)
	{
		auto texture = CreateTexture();
		CreateViews(texture.Get());
	}

	DirectXFramebuffer::DirectXFramebuffer(ID3D11Resource* resource, uint32_t width, uint32_t height, bool depth)
		: mProps({width, height, depth}), mOwnsTexture(false)
	{
		CreateViews(resource);
	}

	void Framebuffer::BindMultiple(const std::vector<Ref<Framebuffer>>& framebuffers) {
		std::vector<ID3D11RenderTargetView*> rtvs;
		ID3D11DepthStencilView* dsv = nullptr;

		uint32_t width, height;

		for (auto& framebuffer : framebuffers) {
			auto dxframebuffer = std::static_pointer_cast<DirectXFramebuffer>(framebuffer);
			rtvs.push_back(dxframebuffer->GetRenderTargetView());
			width = framebuffer->GetWidth();
			height = framebuffer->GetHeight();
			if (!dsv && dxframebuffer->mProps.Depth)
				dsv = dxframebuffer->mDSV.Get();
		}

		auto& context = RetrieveContext();

		D3D11_VIEWPORT vp;
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.Width = (float)width;
		vp.Height = (float)height;

		VOID_CALL(context.GetDeviceContext()->RSSetViewports(1, &vp));
		VOID_CALL(context.GetDeviceContext()->OMSetRenderTargets((uint32_t)rtvs.size(), rtvs.data(), dsv?dsv:nullptr));
	}

	void DirectXFramebuffer::Bind()
	{
		auto& context = RetrieveContext();

		D3D11_VIEWPORT vp;
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.Width = (float)mProps.Width;
		vp.Height = (float)mProps.Height;

		VOID_CALL(context.GetDeviceContext()->RSSetViewports(1, &vp));
		VOID_CALL(context.GetDeviceContext()->OMSetRenderTargets(1, mRTV.GetAddressOf(), mProps.Depth ? mDSV.Get() : nullptr));
	}

	void DirectXFramebuffer::Clear(const float4& color)
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->ClearRenderTargetView(mRTV.Get(), ValuePtr(color)));
		if (mProps.Depth)
			ClearDepth();
	}

	void DirectXFramebuffer::ClearDepth()
	{
		auto& context = RetrieveContext();
		MG_CORE_ASSERT(mProps.Depth, "Cannot clear depth; this framebuffer does not have a depth attachment.");
		VOID_CALL(context.GetDeviceContext()->ClearDepthStencilView(mDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0));
	}

	void DirectXFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		MG_CORE_ASSERT(mOwnsTexture, "Cannot resize a framebuffer if it does not own its texture.");

		mProps.Width = width;
		mProps.Height = height;

		auto texture = CreateTexture();
		CreateViews(texture.Get());
	}

	void DirectXFramebuffer::EnsureSize(uint32_t width, uint32_t height)
	{
		if (mProps.Width != width || mProps.Height != height)
			Resize(width, height);
	}

	void DirectXFramebuffer::BindAsTexture(size_t slot) const
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->PSSetShaderResources((uint32_t)slot, 1, mSRV.GetAddressOf()));
	}

	void DirectXFramebuffer::BindDepthAsTexture(size_t slot) const
	{
		MG_CORE_ASSERT(mProps.Depth, "Cannot bind depth buffer as texture; framebuffer does not own a depth attachment.");

		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->PSSetShaderResources((uint32_t)slot, 1, mDSRV.GetAddressOf()));
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> DirectXFramebuffer::CreateTexture()
	{
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = mProps.Width;
		desc.Height = mProps.Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
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

	void DirectXFramebuffer::CreateViews(ID3D11Resource* resource)
	{
		mResourceReference = resource;
		auto& context = RetrieveContext();
		HR_CALL(context.GetDevice()->CreateRenderTargetView(resource, nullptr, &mRTV));
		if(mOwnsTexture)
			HR_CALL(context.GetDevice()->CreateShaderResourceView(resource, nullptr, &mSRV));

		if (mProps.Depth)
		{
			D3D11_TEXTURE2D_DESC desc = {};
			desc.Width = mProps.Width;
			desc.Height = mProps.Height;
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
			HR_CALL(context.GetDevice()->CreateShaderResourceView(depthTexture.Get(), &sr_desc, &mDSRV));
			HR_CALL(context.GetDevice()->CreateDepthStencilView(depthTexture.Get(), &dsvDesc, &mDSV));
		}
	}

}