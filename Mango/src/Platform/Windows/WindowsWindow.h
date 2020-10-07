#pragma once

#include "Mango/Core/WinCore.h"

#include "Mango/Core/Window.h"

namespace Mango {
	
	class WindowsWindow : public Window {
	public:
		WindowsWindow(const WindowProperties& props);
		~WindowsWindow();

		virtual void OnUpdate() override;

		inline uint32_t GetWidth() override { return mWidth; }
		inline uint32_t GetHeight() override { return mHeight; }
	private:
		void Init();
	private:
		HWND mHandle;

		uint32_t mWidth;
		uint32_t mHeight;
		std::string mTitle;
	};

}