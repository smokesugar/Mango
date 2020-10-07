#pragma once

#include <fstream>

extern Mango::Application* Mango::CreateApplication();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
	#ifdef MG_DEBUG
		AllocConsole();
		SetConsoleTitle(L"Mango Debug Console");
		FILE* fDummy;
		freopen_s(&fDummy, "CONIN$", "r", stdin);
		freopen_s(&fDummy, "CONOUT$", "w", stderr);
		freopen_s(&fDummy, "CONOUT$", "w", stdout);
	#endif

	Mango::Log::Init();

	auto app = Mango::CreateApplication();
	app->Run();
	delete app;
	return 0;
}