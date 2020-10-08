#pragma once

namespace Mango {
	
	class ImGuiContext {
	public:
		static void Init();
		static void Shutdown();

		static void Begin();
		static void End();
	private:
		static void RendererAPI_Init();
		static void WindowAPI_Init();
		static void RendererAPI_Shutdown();
		static void WindowAPI_Shutdown();

		static void WindowAPI_Begin();
		static void RendererAPI_Begin();
		static void RendererAPI_End();
	};

}