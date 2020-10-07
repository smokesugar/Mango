#pragma once

#include <cinttypes>
#include <string>
#include <functional>

namespace Mango {

	struct WindowProperties {
		uint32_t Width;
		uint32_t Height;
		std::string Title;

		WindowProperties()
			: Width(1280), Height(720), Title("Mango Engine")
		{}
		WindowProperties(uint32_t width, uint32_t height, const std::string& title)
			: Width(width), Height(height), Title(title)
		{}
	};
	
	class Window {
	public:
		virtual ~Window() {}
		virtual void OnUpdate() = 0;
		virtual uint32_t GetWidth() = 0;
		virtual uint32_t GetHeight() = 0;

		static Window* Create(const WindowProperties& props = WindowProperties());
	};

}