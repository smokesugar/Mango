#include "mgpch.h"
#include "DirectXContext.h"

#include <examples/imgui_impl_dx11.cpp>

#include "Mango/Core/Application.h"
#include "Mango/ImGui/ImGuiContext.h"
#include "DirectXFramebuffer.h"
#include "DXDebug.h"

namespace Mango {

	void ImGuiContext::RendererAPI_Init() {
		auto& context = RetrieveContext();
		ImGui_ImplDX11_Init(context.GetDevice(), context.GetDeviceContext());
	}

	void ImGuiContext::RendererAPI_Shutdown() {
		ImGui_ImplDX11_Shutdown();
	}

	void ImGuiContext::RendererAPI_Begin() {
		ImGui_ImplDX11_NewFrame();
	}

	void ImGuiContext::RendererAPI_End() {
		auto& context = RetrieveContext();

		auto rtv = std::static_pointer_cast<DirectXFramebuffer>(Application::Get().GetWindow().GetSwapChain().GetFramebuffer())->GetRenderTargetView();
		context.GetDeviceContext()->OMSetRenderTargets(1, &rtv, nullptr);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	GraphicsContext* GraphicsContext::Create() {
		return new DirectXContext();
	}

	DirectXContext::DirectXContext()
	{
		UINT flags = 0;
		#ifdef MG_DEBUG
			DXGIInfoQueue::Init();
			flags |= D3D11_CREATE_DEVICE_DEBUG;
		#endif

		HR_CALL(D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			flags,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&mDevice,
			nullptr,
			&mDeviceContext
		));
	}

	DirectXContext::~DirectXContext()
	{
		#ifdef MG_DEBUG
			DXGIInfoQueue::Shutdown();
		#endif
	}

}