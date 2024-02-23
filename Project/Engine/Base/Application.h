#pragma once
#include <Windows.h>
#include <cstdint>

class Application
{
public:
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

	static Application* GetInstance();

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	void CreateGameWindow(const wchar_t* title, int32_t clientWidth, int32_t clientHeight);

	void CloseGameWindow();

	bool ProcessMessage();

	HWND GetHwnd() const { return hwnd_; };

	HINSTANCE GetHInstance() { return wc_.hInstance; };

private:
	Application() = default;
	~Application() = default;
	Application(const Application&) = delete;
	const Application& operator=(const Application&) = delete;

private:
	WNDCLASS wc_{};

	RECT wrc_{};

	HWND hwnd_{};
};

