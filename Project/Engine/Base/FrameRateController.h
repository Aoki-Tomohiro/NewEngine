#pragma once
#include <chrono>
#include <thread>

class FrameRateController
{
public:
	void Initialize();

	void Update();

private:
	std::chrono::steady_clock::time_point reference_{};
};

