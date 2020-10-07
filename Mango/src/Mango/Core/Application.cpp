#include "mgpch.h"
#include "Application.h"

namespace Mango {

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
        mWindow = Scope<Window>(Window::Create(props));
        mWindow->CreateSwapChain();
    }

    Application::~Application()
    {
        sInstance = nullptr;
        for (auto layer : mLayerStack)
            delete layer;
    }

    void Application::Run()
    {
        while (mRunning) {
            mWindow->OnUpdate();

            for (auto layer : mLayerStack)
                layer->OnUpdate();

            mWindow->GetSwapChain().Present();
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