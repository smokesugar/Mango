#pragma once

extern Mango::Application* Mango::CreateApplication();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
	auto app = Mango::CreateApplication();
	app->Run();
	delete app;
	return 0;
}