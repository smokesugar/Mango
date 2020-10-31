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

		CreateInternalFramebuffer(width, height);
	}


	void DirectXSwapChain::Present()
	{
		HR_CALL(mInternal->Present(0, 0));
	}

	Ref<ColorBuffer> DirectXSwapChain::GetFramebuffer()
	{
		return std::static_pointer_cast<ColorBuffer>(mFramebuffer);
	}

	void DirectXSwapChain::Resize(uint32_t width, uint32_t height)
	{
		mFramebuffer.reset();
		HR_CALL(mInternal->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0));
		CreateInternalFramebuffer(width, height);
	}

	void DirectXSwapChain::CreateInternalFramebuffer(uint32_t width, uint32_t height)
	{
		Microsoft::WRL::ComPtr<ID3D11Resource> backbuffer;
		HR_CALL(mInternal->GetBuffer(0, __uuidof(ID3D11Resource), &backbuffer));
		mFramebuffer = CreateRef<DirectXColorBuffer>(backbuffer.Get(), width, height);
	}

}