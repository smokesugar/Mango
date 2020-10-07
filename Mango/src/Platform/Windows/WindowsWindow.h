#pragma once

#include "Mango/Core/WinCore.h"

#include "Mango/Core/Window.h"

namespace Mango {
	
	class WindowsWindow : public Window {
	public:
		WindowsWindow(const WindowProperties& props);
		~WindowsWindow();

		virtual void OnUpdate() override;

		inline uint32_t GetWidth() const override { return mWidth; }
		inline uint32_t GetHeight() const override { return mHeight; }

		virtual void* GetNativeWindow() override { return mHandle; }
	private:
		void Init();
		static LRESULT CALLBACK _WindowProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam);
		LRESULT WindowProc(UINT msg, WPARAM wparam, LPARAM lparam);
	private:
		HWND mHandle;

		uint32_t mWidth;
		uint32_t mHeight;
		std::string mTitle;
		EventCallback mEventCallback;
	};

}