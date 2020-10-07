#pragma once

#include "Base.h"
#include "Window.h"

namespace Mango {
	
	class Application {
	public:
		Application(const std::string& title = "Mango Application");
		virtual ~Application() {}

		void Run();

		inline Window& GetWindow() { return *mWindow; }
	private:
		Scope<Window> mWindow;
	};

	static Application* CreateApplication();

}