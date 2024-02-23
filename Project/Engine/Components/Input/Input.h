#pragma once
#include <memory>
#include <wrl.h>
#define DIRECTINPUT_VERSION 0x0800//DirectInputのバージョン指定
#include <dinput.h>
#include <Xinput.h>
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"xinput.lib")

class Input {
private:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	static Input* GetInstance();

	static void Destroy();

	void Initialize();

	void Update();

	bool IsPushKey(uint8_t keyNum);

	bool IsPushKeyEnter(uint8_t keyNum);

	bool IsPushKeyExit(uint8_t keyNum);

	bool IsPressMouse(int32_t mouseNum);

	bool IsPressMouseEnter(int32_t mouseNum);

	bool IsPressMouseExit(int32_t mouseNum);

	int32_t GetWheel();

	bool IsControllerConnected();

	bool IsPressButton(WORD button);

	bool IsPressButtonEnter(WORD button);

	bool IsPressButtonExit(WORD button);

	float GetLeftStickX();

	float GetLeftStickY();

	float GetRightStickX();

	float GetRightStickY();

private:
	Input() = default;
	~Input() = default;
	Input(const Input&) = delete;
	const Input& operator = (const Input&) = delete;

private:
	static Input* instance_;

	ComPtr<IDirectInput8> directInput_ = nullptr;

	ComPtr<IDirectInputDevice8> keyboardDevice_ = nullptr;

	ComPtr<IDirectInputDevice8> mouseDevice_ = nullptr;

	BYTE key_[256] = {};

	BYTE preKey_[256] = {};

	DIMOUSESTATE mouse_ = {};

	DIMOUSESTATE mousePre_ = {};

	XINPUT_STATE state_{};

	XINPUT_STATE preState_{};
};

