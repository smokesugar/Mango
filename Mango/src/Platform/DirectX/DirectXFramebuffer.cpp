#include "mgpch.h"
#include "DirectXFramebuffer.h"

#include "Mango/Core/Application.h"
#include "DirectXContext.h"

namespace Mango {

	Framebuffer* Framebuffer::Create(const FramebufferProperties& props) {
		return new DirectXFramebuffer(props);
	}

	DirectXFramebuffer::DirectXFramebuffer(const FramebufferProperties& props)
		: mProps(props), mOwnsTexture(true)
	{
		auto texture = CreateTexture();
		CreateRenderTargetView(texture.Get());
	}

	DirectXFramebuffer::DirectXFramebuffer(ID3D11Resource* resource, uint32_t width, uint32_t height)
		: mProps({width, height}), mOwnsTexture(false)
	{
		CreateRenderTargetView(resource);
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
		VOID_CALL(context.GetDeviceContext()->OMSetRenderTargets(1, mRTV.GetAddressOf(), nullptr));
	}

	void DirectXFramebuffer::Clear(const float4& color)
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->ClearRenderTargetView(mRTV.Get(), ValuePtr(color)));
	}

	void DirectXFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		MG_CORE_ASSERT(mOwnsTexture, "Cannot resize a framebuffer if it does not own its texture.");

		mProps.Width = width;
		mProps.Height = height;

		auto texture = CreateTexture();
		CreateRenderTargetView(texture.Get());
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> DirectXFramebuffer::CreateTexture()
	{
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = mProps.Width;
		desc.Height = mProps.Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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

	void DirectXFramebuffer::CreateRenderTargetView(ID3D11Resource* resource)
	{
		auto& context = RetrieveContext();
		HR_CALL(context.GetDevice()->CreateRenderTargetView(resource, nullptr, &mRTV));
	}

}