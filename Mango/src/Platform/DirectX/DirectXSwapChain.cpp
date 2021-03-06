#include "mgpch.h"
#include "DirectXSwapChain.h"

#include "Mango/Core/Application.h"
#include "DirectXContext.h"

namespace Mango {

	SwapChain* SwapChain::Create() {
		return new DirectXSwapChain();
	}

	DirectXSwapChain::DirectXSwapChain()
	{
		uint32_t width = Application::Get().GetWindow().GetWidth();
		uint32_t height = Application::Get().GetWindow().GetHeight();

		auto& context = RetrieveContext();

		Microsoft::WRL::ComPtr<IDXGIFactory> factory;
		HR_CALL(CreateDXGIFactory(__uuidof(IDXGIFactory), &factory));

		DXGI_SWAP_CHAIN_DESC desc = {};
		desc.BufferDesc.Width = width;
		desc.BufferDesc.Height = height;
		desc.BufferDesc.RefreshRate = {0, 0};
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.SampleDesc = { 1, 0 };
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = 2;
		desc.OutputWindow = (HWND)Application::Get().GetWindow().GetNativeWindow();
		desc.Windowed = true;
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		desc.Flags = 0;

		HR_CALL(factory->CreateSwapChain(context.GetDevice(), &desc, &mInternal));

		CreateRenderTargetView(width, height);
	}


	void DirectXSwapChain::Present()
	{
		HR_CALL(mInternal->Present(0, 0));
	}

	void DirectXSwapChain::BindAsRenderTarget()
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->OMSetRenderTargets(1, mRTV.GetAddressOf(), nullptr));
	}

	void DirectXSwapChain::Resize(uint32_t width, uint32_t height)
	{
		mRTV.Reset();
		mBackbuffer.Reset();
		HR_CALL(mInternal->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0));
		CreateRenderTargetView(width, height);
	}

	void DirectXSwapChain::CreateRenderTargetView(uint32_t width, uint32_t height)
	{
		auto& context = RetrieveContext();

		HR_CALL(mInternal->GetBuffer(0, __uuidof(ID3D11Resource), &mBackbuffer));
		HR_CALL(context.GetDevice()->CreateRenderTargetView(mBackbuffer.Get(), nullptr, &mRTV));
	}

}