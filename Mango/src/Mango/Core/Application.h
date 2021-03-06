#pragma once

#include "Base.h"
#include "Window.h"
#include "Layer.h"
#include "Mango/Events/Events.h"
#include "Mango/Renderer/GraphicsContext.h"
#include "RuntimeLog.h"

namespace Mango {
	
	class Application {
	public:
		Application(const std::string& title = "Mango Application");
		virtual ~Application();

		void Run();

		inline static Application& Get() { return *sInstance; }

		inline Window& GetWindow() { return *mWindow; }
		inline GraphicsContext& GetGraphicsContext() { return *mGraphicsContext; }
		inline RuntimeLog& GetRuntimeLog() { return mRuntimeLog; }

		inline void Close() { mRunning = false; }
	protected:
		void PushLayer(Layer* layer);
	private:
		void EventCallback(Event& e);
		bool OnWindowClose(WindowCloseEvent& e);
	private:
		static Application* sInstance;
		Scope<Window> mWindow;
		Scope<GraphicsContext> mGraphicsContext;
		std::vector<Layer*> mLayerStack;
		RuntimeLog mRuntimeLog;
		bool mRunning = true;
	};

	static Application* CreateApplication();

}