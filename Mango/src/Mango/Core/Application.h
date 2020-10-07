#pragma once

#include "Base.h"
#include "Window.h"
#include "Mango/Events/Events.h"

namespace Mango {
	
	class Application {
	public:
		Application(const std::string& title = "Mango Application");
		virtual ~Application() {}

		void Run();

		inline Window& GetWindow() { return *mWindow; }
	private:
		void EventCallback(Event& e);
		bool OnWindowClose(WindowCloseEvent& e);
	private:
		Scope<Window> mWindow;
		bool mRunning = true;
	};

	static Application* CreateApplication();

}