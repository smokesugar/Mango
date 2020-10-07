#pragma once

#include <cinttypes>
#include <string>
#include <functional>

#include "Mango/Events/Events.h"

namespace Mango {

	using EventCallback = std::function<void(Event& e)>;

	struct WindowProperties {
		uint32_t Width;
		uint32_t Height;
		std::string Title;
		EventCallback EventFn;

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
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		static Window* Create(const WindowProperties& props = WindowProperties());

		virtual void* GetNativeWindow() = 0;
	};

}