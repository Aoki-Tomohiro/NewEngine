#include "RandomGenerator.h"

std::mt19937 RandomGenerator::randomEngine_;

void RandomGenerator::Initialize() 
{
	//ランダムエンジンの初期化
	std::random_device seedGenerator;
	randomEngine_ = std::mt19937(seedGenerator());
}

int RandomGenerator::GetRandomInt(int min, int max)
{
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(randomEngine_);
}

float RandomGenerator::GetRandomFloat(float min, float max) 
{
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(randomEngine_);
}