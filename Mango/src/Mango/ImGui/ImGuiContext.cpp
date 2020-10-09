#include "mgpch.h"
#include "ImGuiContext.h"

#include <imgui.h>

namespace Mango {

	void ImGuiContext::Init()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		RendererAPI_Init();
		WindowAPI_Init();
	}

	void ImGuiContext::Shutdown()
	{
		RendererAPI_Shutdown();
		WindowAPI_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiContext::Begin()
	{
		RendererAPI_Begin();
		WindowAPI_Begin();
		ImGui::NewFrame();
	}

	void ImGuiContext::End()
	{
		ImGui::Render();
		RendererAPI_End();
	}

}