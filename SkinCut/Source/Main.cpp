/**
* Author: Maurits Lam
*
* Build requirements:
*	Visual Studio 2022
*	Windows SDK 10/11
*/


#include "Application.hpp"

#include <cstdlib>
#include <sstream>
#include <filesystem>

#include <io.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <windows.h>


SkinCut::Application gApp;

static const uint32_t cWindowWidth = 1280;
static const uint32_t cWindowHeight = 720;


static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return gApp.WndProc(hWnd, msg, wParam, lParam);
}



static bool Initialize(HWND hwnd)
{
	CHAR exeFilePath[MAX_PATH] = { 0 };
	::GetModuleFileNameA(NULL, &exeFilePath[0], MAX_PATH);
    std::string executablePath = std::filesystem::path(std::string(exeFilePath)).remove_filename().string();

	// Default resource directory
	std::string resourcePath = std::string(executablePath) + "Resources\\";

	// Custom resource directory
	if (__argc > 1) {
		resourcePath = (__argv[1]);
		if (resourcePath.back() != '\\') {
			resourcePath.append("\\");
		}
	}
	
	// Attempt to find resource directory
	if (::GetFileAttributesA(resourcePath.c_str()) == INVALID_FILE_ATTRIBUTES) {
		std::stringstream ss;
		ss << "Unable to locate resource directory '" << resourcePath << "'";
		::MessageBoxA(nullptr, ss.str().c_str(), "ERROR", MB_ICONERROR | MB_OK);
		return false;
	}

	
	// Attempt to find config and scene descriptions
	WIN32_FIND_DATAA data;
	std::string configFile = resourcePath + std::string("Config.json");
	std::string sceneFile = resourcePath + std::string("Scene.json");

	if (::FindFirstFileA(configFile.c_str(), &data) == INVALID_HANDLE_VALUE) {
		::MessageBoxA(nullptr, "Unable to locate config file", "ERROR", MB_ICONERROR | MB_OK);
		return false;
	}

	if (::FindFirstFileA(sceneFile.c_str(), &data) == INVALID_HANDLE_VALUE) {
		::MessageBoxA(nullptr, "Unable to locate scene file", "ERROR", MB_ICONERROR | MB_OK);
		return false;
	}

	// Attempt to find shader directory
	std::string shaderDir = resourcePath + std::string("Shaders\\");

	if (::GetFileAttributesA(shaderDir.c_str()) == INVALID_FILE_ATTRIBUTES) {
		::MessageBoxA(nullptr, "Unable to locate shader directory", "ERROR", MB_ICONERROR | MB_OK);
		return false;
	}

	// Attempt to find texture directory
	std::string textureDir = resourcePath + std::string("Textures\\");

	if (::GetFileAttributesA(textureDir.c_str()) == INVALID_FILE_ATTRIBUTES) {
		::MessageBoxA(nullptr, "Unable to locate texture directory", "ERROR", MB_ICONERROR | MB_OK);
		return false;
	}

	// Attempt to find font directory
	std::string fontDir = resourcePath + std::string("Fonts\\");

	if (GetFileAttributesA(fontDir.c_str()) == INVALID_FILE_ATTRIBUTES) {
		::MessageBoxA(nullptr, "Unable to locate font directory", "ERROR", MB_ICONERROR | MB_OK);
		return false;
	}

	// Attempt initialization
	if (!gApp.Initialize(hwnd, resourcePath)) {
		::MessageBoxA(nullptr, "Initialization failed", "ERROR", MB_ICONERROR | MB_OK);
		return false;
	}

	return true;
}


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR szCmdLine, _In_ int iCmdShow)
{
#ifdef _DEBUG
	// Track memory allocation
	::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	::_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif


	// Allocate window and redirect output there
	::AllocConsole();
	FILE* pCout = nullptr;
	FILE* pCerr = nullptr;
	std::ignore = freopen_s(&pCout, "CONOUT$", "w", stdout);
	std::ignore = freopen_s(&pCerr, "CONOUT$", "w", stderr);

		
	// Create window class
	WNDCLASSEX wcex{};
	wcex.lpszClassName = L"WINDOW_CLASS";
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = &WndProc;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wcex.hIconSm = nullptr;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = nullptr;
	wcex.lpszMenuName = nullptr;
	wcex.cbClsExtra	= 0;
	wcex.cbWndExtra = 0;

	if (!RegisterClassEx(&wcex)) {
		::MessageBoxA(nullptr, "Window registration failed", "ERROR", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	// Determine the position of the window.
	RECT rectTaskbar;
	LONG taskbarW = 0L;
	LONG taskbarH = 0L;
	HWND hwndTaskbar = FindWindow(L"Shell_traywnd", nullptr); // find taskbar

	if (hwndTaskbar && ::GetWindowRect(hwndTaskbar, &rectTaskbar)) {
		taskbarW = rectTaskbar.right - rectTaskbar.left;
		taskbarH = rectTaskbar.bottom - rectTaskbar.top;

		if (taskbarW > taskbarH) { // taskbar on top or bottom
			taskbarW = 0;
			if (rectTaskbar.top == 0) {
				taskbarH = -taskbarH; // taskbar on top
			}
		}
		else { // taskbar on left or right
			taskbarH = 0;
			if (rectTaskbar.left == 0) {
				taskbarW = -taskbarW; // taskbar on left
			}
		}
	}

	int windowX = (::GetSystemMetrics(SM_CXSCREEN) - cWindowWidth  - taskbarW) / 2;
	int windowY = (::GetSystemMetrics(SM_CYSCREEN) - cWindowHeight - taskbarH) / 2;

	// Create window
	HWND hWnd = CreateWindowEx(WS_EX_CLIENTEDGE, L"WINDOW_CLASS", L"SkinCut", WS_OVERLAPPEDWINDOW, 
		windowX, windowY, cWindowWidth, cWindowHeight, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd) {
		::MessageBoxA(nullptr, "Window creation failed", "ERROR", MB_ICONERROR | MB_OK);
		return 1;
	}
	
	if (!Initialize(hWnd)) {
		::MessageBoxA(nullptr, "Initialization failed", "ERROR", MB_ICONERROR | MB_OK);
		return 1;
	}

	// Show window
	::ShowWindow(hWnd, SW_SHOWDEFAULT);
	::UpdateWindow(hWnd);

	MSG msg;
	::ZeroMemory(&msg, sizeof(MSG));

	while (msg.message != WM_QUIT) {
		if (::PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
			continue;
		}

		try {
			gApp.Update();
			gApp.Render();
		}
		catch (std::exception& e) {
			std::stringstream ss;
			ss << "Critical error: " << e.what();
			::MessageBoxA(nullptr, ss.str().c_str(), "ERROR", MB_ICONERROR | MB_OK);

			::ChangeDisplaySettingsW(nullptr, 0);
			::DestroyWindow(hWnd);
			::UnregisterClassW(wcex.lpszClassName, wcex.hInstance);
			return 1;
		}
	}
	
	::ChangeDisplaySettingsW(nullptr, 0);
	::DestroyWindow(hWnd);
	::UnregisterClassW(wcex.lpszClassName, wcex.hInstance);

	return static_cast<int>(msg.wParam);
}
