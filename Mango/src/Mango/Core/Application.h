#pragma once

namespace Mango {
	
	class Application {
	public:
		virtual ~Application() {}

		void Run();
	};

	static Application* CreateApplication();

}