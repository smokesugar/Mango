#include "mgpch.h"
#include "Application.h"

namespace Mango {

    Application::Application(const std::string& title)
    {
        WindowProperties props = {};
        props.Width = 1280;
        props.Height = 720;
        props.Title = title;
        props.EventFn = MG_BIND_FN(Application::EventCallback);
        mWindow = Scope<Window>(Window::Create(props));
    }

    void Application::Run()
    {
        while (mRunning) {
            mWindow->OnUpdate();
        }
    }

    void Application::EventCallback(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(MG_BIND_FN(Application::OnWindowClose));
    }

    bool Application::OnWindowClose(WindowCloseEvent& e)
    {
        mRunning = false;
        return false;
    }

}