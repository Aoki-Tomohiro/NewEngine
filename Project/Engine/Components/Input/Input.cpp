#include "Input.h"
#include "Engine/Base/Application.h"
#include <cassert>

Input* Input::instance_ = nullptr;

Input* Input::GetInstance() 
{
	if (instance_ == nullptr) {
		instance_ = new Input();
	}
	return instance_;
}

void Input::Destroy()
{
	if (instance_)
	{
		delete instance_;
		instance_ = nullptr;
	}
}

void Input::Initialize()
{
	//Applicationのインスタンスを取得
	Application* application = Application::GetInstance();

	HRESULT result;
	//DirectInputの初期化
	HINSTANCE hinstance = application->GetHInstance();
	result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput_, nullptr);
	assert(SUCCEEDED(result));

	//キーボードデバイスの生成
	result = directInput_->CreateDevice(GUID_SysKeyboard, &keyboardDevice_, NULL);
	assert(SUCCEEDED(result));
	//マウスデバイスの生成
	result = directInput_->CreateDevice(GUID_SysMouse, &mouseDevice_, NULL);
	assert(SUCCEEDED(result));

	//キーボード入力データ形式のセット
	result = keyboardDevice_->SetDataFormat(&c_dfDIKeyboard);//標準形式
	assert(SUCCEEDED(result));
	//マウス入力データ形式のセット
	result = mouseDevice_->SetDataFormat(&c_dfDIMouse);
	assert(SUCCEEDED(result));

	//排他制御レベルのセット
	HWND hwnd = application->GetHwnd();
	result = keyboardDevice_->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
	result = mouseDevice_->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(result));

	//コントローラーデバイスの初期化
	ZeroMemory(&state_, sizeof(XINPUT_STATE));
	ZeroMemory(&preState_, sizeof(XINPUT_STATE));
}

void Input::Update() 
{
	//前のフレームのキーボード入力を取得する
	std::memcpy(preKey_, key_, 256);
	//前のフレームのマウス入力を取得する
	mousePre_ = mouse_;
	//前のフレームのコントローラー入力を取得する
	preState_ = state_;

	//キーボード情報の取得開始
	keyboardDevice_->Acquire();
	//マウス情報の取得開始
	mouseDevice_->Acquire();

	//全キーの入力状態を取得する
	keyboardDevice_->GetDeviceState(sizeof(key_), key_);
	//マウスの入力状態を取得する
	mouseDevice_->GetDeviceState(sizeof(DIMOUSESTATE), &mouse_);

	//コントローラーの状態を取得
	DWORD dwResult = XInputGetState(0, &state_);
	if (dwResult == ERROR_SUCCESS) 
	{
		ZeroMemory(&state_, sizeof(XINPUT_STATE));
	}
}

bool Input::IsPushKey(uint8_t keyNum)
{
	if (key_[keyNum] == 0x80) {
		return true;
	}
	return false;
}

bool Input::IsPushKeyEnter(uint8_t keyNum)
{
	if (key_[keyNum] == 0x80 && preKey_[keyNum] == 0x00) {
		return true;
	}
	return false;
}

bool Input::IsPushKeyExit(uint8_t keyNum) 
{
	if (key_[keyNum] == 0x00 && preKey_[keyNum] == 0x80) {
		return true;
	}
	return false;
}

bool Input::IsPressMouse(int32_t mouseNum)
{
	if (mouse_.rgbButtons[mouseNum] == 0x80) {
		return true;
	}
	return false;
}

bool Input::IsPressMouseEnter(int32_t mouseNum)
{
	if (mouse_.rgbButtons[mouseNum] == 0x80 && mousePre_.rgbButtons[mouseNum] == 0x00) {
		return true;
	}
	return false;
}

bool Input::IsPressMouseExit(int32_t mouseNum)
{
	if (mouse_.rgbButtons[mouseNum] == 0x00 && mousePre_.rgbButtons[mouseNum] == 0x80) {
		return true;
	}
	return false;
}

int32_t Input::GetWheel() 
{
	return mouse_.lZ;
}

bool Input::IsControllerConnected()
{
	DWORD dwResult = XInputGetState(0, &state_);
	if (dwResult == ERROR_SUCCESS) {
		return true;
	}
	return false;
}

bool Input::IsPressButton(WORD button) 
{
	if (state_.Gamepad.wButtons & button) {
		return true;
	}
	return false;
}

bool Input::IsPressButtonEnter(WORD button) 
{
	if ((state_.Gamepad.wButtons & button) && !(preState_.Gamepad.wButtons & button)) {
		return true;
	}
	return false;
}

bool Input::IsPressButtonExit(WORD button) 
{
	if (!(state_.Gamepad.wButtons & button) && (preState_.Gamepad.wButtons & button)) {
		return true;
	}
	return false;
}

float Input::GetLeftStickX()
{
	float leftStickXValue = static_cast<float>(state_.Gamepad.sThumbLX) / SHRT_MAX;
	return leftStickXValue;
}

float Input::GetLeftStickY()
{
	float leftStickYValue = static_cast<float>(state_.Gamepad.sThumbLY) / SHRT_MAX;
	return leftStickYValue;
}

float Input::GetRightStickX()
{
	float rightStickXValue = static_cast<float>(state_.Gamepad.sThumbRX) / SHRT_MAX;
	return rightStickXValue;
}

float Input::GetRightStickY()
{
	float rightStickYValue = static_cast<float>(state_.Gamepad.sThumbRY) / SHRT_MAX;
	return rightStickYValue;
}