#include "mgpch.h"
#include "Application.h"
#include "Mango/Renderer/Render/Renderer.h"
#include "Mango/ImGui/ImGuiContext.h"

namespace Mango {

    static double GetTime() {
		LARGE_INTEGER time, freq;
		QueryPerformanceCounter(&time);
		QueryPerformanceFrequency(&freq);

		return (double)time.QuadPart / (double)freq.QuadPart;
    }

    Application* Application::sInstance = nullptr;

    Application::Application(const std::string& title)
    {
        MG_CORE_ASSERT(!sInstance, "Only one application can exist at any given time.");
        sInstance = this;

        mGraphicsContext = Scope<GraphicsContext>(GraphicsContext::Create());

        WindowProperties props = {};
        props.Width = 1280;
        props.Height = 720;
        props.Title = title;
        props.EventFn = MG_BIND_FN(Application::EventCallback);
        props.Maximised = true;
        mWindow = Scope<Window>(Window::Create(props));
        mWindow->CreateSwapChain();

		ImGuiContext::Init();
		Renderer::Init();
    }

    Application::~Application()
	{
		ImGuiContext::Shutdown();
		Renderer::Shutdown();

        sInstance = nullptr;
        for (auto layer : mLayerStack)
            delete layer;
    }

    void Application::Run()
    {
        while (mRunning) {
            static double lastTime = GetTime();
            double time = GetTime();
            double deltaTime = time - lastTime;
            lastTime = time;

            for (auto layer : mLayerStack)
                layer->OnUpdate((float)deltaTime);

			ImGuiContext::Begin();
			for (auto layer : mLayerStack)
				layer->OnImGuiRender();
			ImGuiContext::End();

            mWindow->GetSwapChain().Present();

            mWindow->OnUpdate();
        }
    }

    void Application::PushLayer(Layer* layer)
    {
        mLayerStack.push_back(layer);
    }

    void Application::EventCallback(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(MG_BIND_FN(Application::OnWindowClose));

        for (auto it = mLayerStack.rbegin(); it != mLayerStack.rend(); it++) {
            if (e.Handled)
                break;
            (*it)->OnEvent(e);
        }
    }

    bool Application::OnWindowClose(WindowCloseEvent& e)
    {
        mRunning = false;
        return false;
    }

}