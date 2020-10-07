#include "mgpch.h"
#include "Application.h"

namespace Mango {

    Application::Application(const std::string& title)
    {
        mWindow = Scope<Window>(Window::Create({1280, 720, title}));
    }

    void Application::Run()
    {
        while (true) {
            mWindow->OnUpdate();
        }
    }

}