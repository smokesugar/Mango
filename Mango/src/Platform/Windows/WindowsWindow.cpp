#include "mgpch.h"
#include "WindowsWindow.h"

#include "Mango/Core/Base.h"

namespace Mango {

	static std::wstring convert(const std::string& as)
	{
		if (as.empty())   
			return std::wstring();
		size_t reqLength = ::MultiByteToWideChar(CP_UTF8, 0, as.c_str(), (int)as.length(), 0, 0);
		std::wstring ret(reqLength, L'\0');
		::MultiByteToWideChar(CP_UTF8, 0, as.c_str(), (int)as.length(), &ret[0], (int)ret.length());
		return ret;
	}

	Window* Window::Create(const WindowProperties& props) {
		return new WindowsWindow(props);
	}

	WindowsWindow::WindowsWindow(const WindowProperties& props)
		: mWidth(props.Width), mHeight(props.Height), mTitle(props.Title),
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

	void WindowsWindow::Init()
	{
		LPWSTR window_class = L"mangoWindowClass";

		HINSTANCE hInstance = GetModuleHandle(nullptr);

		WNDCLASS wc = {};
		wc.lpfnWndProc = DefWindowProc;
		wc.hInstance = hInstance;
		wc.lpszClassName = window_class;
		RegisterClass(&wc);

		RECT wr;
		wr.left = 100;
		wr.right = mWidth + wr.left;
		wr.top = 100;
		wr.bottom = mHeight + wr.top;
		AdjustWindowRectEx(&wr, WS_OVERLAPPEDWINDOW, FALSE, 0);

		mHandle = CreateWindowEx(0, window_class, convert(mTitle).c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, nullptr, nullptr, hInstance, this);
	
		MG_CORE_ASSERT(mHandle, "Failed to create window: {0}", Translate(GetLastError()));

		ShowWindow(mHandle, SW_SHOWDEFAULT);

		UnregisterClass(window_class, hInstance);
	}

}