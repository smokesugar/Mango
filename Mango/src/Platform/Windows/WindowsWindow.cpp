#include "mgpch.h"
#include "WindowsWindow.h"

#include <examples/imgui_impl_win32.h>
#include <examples/imgui_impl_win32.cpp>

#include "Mango/Core/Base.h"
#include "Mango/Core/Application.h"
#include "Mango/ImGui/ImGuiContext.h"

namespace Mango {

	void ImGuiContext::WindowAPI_Init() {
		ImGui_ImplWin32_Init(Application::Get().GetWindow().GetNativeWindow());
	}

	void ImGuiContext::WindowAPI_Shutdown() {
		ImGui_ImplWin32_Shutdown();
	}

	void ImGuiContext::WindowAPI_Begin() {
		ImGui_ImplWin32_NewFrame();
	}

	Window* Window::Create(const WindowProperties& props) {
		return new WindowsWindow(props);
	}

	WindowsWindow::WindowsWindow(const WindowProperties& props)
		: mWidth(props.Width), mHeight(props.Height), mTitle(props.Title), mEventCallback(props.EventFn),
		mHandle(nullptr)
	{
		Init();
	}

	WindowsWindow::~WindowsWindow()
	{
		DestroyWindow(mHandle);
	}

	void WindowsWindow::OnUpdate()
	{
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	LRESULT CALLBACK WindowsWindow::_WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		WindowsWindow* window;

		if (msg == WM_NCCREATE) {
			CREATESTRUCT* pCreate = (CREATESTRUCT*)lparam;
			window = (WindowsWindow*)pCreate->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);

			window->mHandle = hwnd;
		}
		else {
			window = (WindowsWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		}

		if (window)
			return window->WindowProc(msg, wparam, lparam);
		else
			return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	LRESULT WindowsWindow::WindowProc(UINT msg, WPARAM wparam, LPARAM lparam)
	{
		ImGui_ImplWin32_WndProcHandler(mHandle, msg, wparam, lparam);

		switch (msg) {
			// Application Events
		case WM_CLOSE: {
			WindowCloseEvent e;
			mEventCallback(e);
			break;
		}
		case WM_SIZE: {
			if (mHandle) {
				mWidth = LOWORD(lparam);
				mHeight = HIWORD(lparam);
				if(mSwapChain)
					mSwapChain->Resize(mWidth, mHeight);
				WindowResizeEvent e(mWidth, mHeight);
				mEventCallback(e);
			}
			break;
		}

					// Keyboard Events
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN: {
			KeyDownEvent e((uint32_t)wparam);
			mEventCallback(e);
			break;
		}
		case WM_SYSKEYUP:
		case WM_KEYUP: {
			KeyUpEvent e((uint32_t)wparam);
			mEventCallback(e);
			break;
		}

					 // Mouse Events
		case WM_MOUSEMOVE: {
			auto pos = MAKEPOINTS(lparam);
			MouseMoveEvent e((float)pos.x, (float)pos.y);
			mEventCallback(e);
			break;
		}
		case WM_LBUTTONDOWN: {
			MouseButtonDownEvent e(VK_LBUTTON);
			mEventCallback(e);
			break;
		}
		case WM_LBUTTONUP: {
			MouseButtonUpEvent e(VK_LBUTTON);
			mEventCallback(e);
			break;
		}
		case WM_MBUTTONDOWN: {
			MouseButtonDownEvent e(VK_MBUTTON);
			mEventCallback(e);
			break;
		}
		case WM_MBUTTONUP: {
			MouseButtonUpEvent e(VK_MBUTTON);
			mEventCallback(e);
			break;
		}
		case WM_RBUTTONDOWN: {
			MouseButtonDownEvent e(VK_RBUTTON);
			mEventCallback(e);
			break;
		}
		case WM_RBUTTONUP: {
			MouseButtonUpEvent e(VK_RBUTTON);
			mEventCallback(e);
			break;
		}

		case WM_MOUSEWHEEL: {
			static int accum = 0;
			accum += GET_WHEEL_DELTA_WPARAM(wparam);

			while (accum >= WHEEL_DELTA)
			{
				MouseScrollEvent e(1);
				mEventCallback(e);
				accum -= WHEEL_DELTA;
			}
			while (accum <= -WHEEL_DELTA)
			{
				MouseScrollEvent e(-1);
				mEventCallback(e);
				accum += WHEEL_DELTA;
			}

			break;
		}

		}

		return DefWindowProc(mHandle, msg, wparam, lparam);
	}

	void WindowsWindow::Init()
	{
		LPWSTR window_class = L"mangoWindowClass";

		HINSTANCE hInstance = GetModuleHandle(nullptr);

		WNDCLASS wc = {};
		wc.lpfnWndProc = _WindowProc;
		wc.hInstance = hInstance;
		wc.lpszClassName = window_class;
		RegisterClass(&wc);

		RECT wr;
		wr.left = 100;
		wr.right = mWidth + wr.left;
		wr.top = 100;
		wr.bottom = mHeight + wr.top;
		AdjustWindowRectEx(&wr, WS_OVERLAPPEDWINDOW, FALSE, 0);

		mHandle = CreateWindowEx(0, window_class, WidenString(mTitle).c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, nullptr, nullptr, hInstance, this);

		MG_CORE_ASSERT(mHandle, "Failed to create window: {0}", Translate(GetLastError()));

		ShowWindow(mHandle, SW_SHOWDEFAULT);

		UnregisterClass(window_class, hInstance);

		MG_CORE_INFO("Created a window: {0} ({1}, {2})", mTitle, mWidth, mHeight);
	}

}