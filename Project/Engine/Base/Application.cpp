#include "Application.h"
#include "Engine/Externals//imgui/imgui_impl_win32.h"
#pragma comment(lib,"winmm.lib")
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

Application* Application::GetInstance() {
	static Application instance;
	return &instance;
}

LRESULT CALLBACK Application::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	//ImGuiにメッセージを伝える
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}

	//メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
		//ウィンドウが破棄された
	case WM_DESTROY:
		//OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	//標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void Application::CreateGameWindow(const wchar_t* title, int32_t clientWidth, int32_t clientHeight) {
	//COM初期化
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

	//システムタイマーの分解能をあげる
	timeBeginPeriod(1);

	//ウィンドウプロシージャ
	wc_.lpfnWndProc = WindowProc;
	//ウィンドウクラス名
	wc_.lpszClassName = L"CG2WindowClass";
	//インスタンスハンドル
	wc_.hInstance = GetModuleHandle(nullptr);
	//カーソル
	wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);
	//ウィンドウクラスを登録する
	RegisterClass(&wc_);


	//ウィンドウサイズを表す構造体にクライアント領域を入れる
	wrc_ = { 0,0,clientWidth,clientHeight };
	//クライアント領域をもとに実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc_, WS_OVERLAPPEDWINDOW, false);


	//ウィンドウの作成
	hwnd_ = CreateWindow(
		wc_.lpszClassName,//利用するクラス名
		title,//タイトルバーの文字
		WS_OVERLAPPEDWINDOW,//ウィンドウスタイル
		CW_USEDEFAULT,//表示X座標
		CW_USEDEFAULT,//表示Y座標
		wrc_.right - wrc_.left,//ウィンドウの横幅
		wrc_.bottom - wrc_.top,//ウィンドウの縦幅
		nullptr,//親ウィンドウハンドル
		nullptr,//メニューハンドル
		wc_.hInstance,//インスタンスハンドル
		nullptr);
	//ウィンドウを表示する
	ShowWindow(hwnd_, SW_SHOW);
}

void Application::CloseGameWindow() {
	//ゲームウィンドウを閉じる
	CloseWindow(hwnd_);
	//COM終了
	CoUninitialize();
}

bool Application::ProcessMessage() {
	//メッセージ
	MSG msg{};

	//Windowにメッセージが来てたら最優先で処理させる
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	//終了メッセージが来たらループを抜ける
	if (msg.message == WM_QUIT) {
		return true;
	}

	return false;
}