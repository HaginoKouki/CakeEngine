#include <Windows.h>

#include "Engine/Application/Application.h"

// Windowsアプリでのエントリーポイント(main関数).
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	Cake::Application app;
	app.Initialize(hInstance);
	app.Run();
	return 0;
}
