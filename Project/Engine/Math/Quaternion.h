#pragma once
#include <cmath>
#include <limits>

struct Quaternion
{
	float x;
	float y;
	float z;
	float w;

	Quaternion operator+(const Quaternion rhs) const
	{
		return Quaternion(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
	}

	Quaternion operator-(const Quaternion rhs) const
	{
		return Quaternion(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
	}

	Quaternion operator*(const Quaternion rhs) const
	{
		return Quaternion(
			w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
			w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,
			w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w,
			w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z
		);
	}

	Quaternion operator+=(const Quaternion rhs)
	{
		*this = *this + rhs;
		return *this;
	}

	Quaternion operator-=(const Quaternion rhs)
	{
		*this = *this - rhs;
		return *this;
	}

	Quaternion operator*=(const Quaternion rhs)
	{
		*this = *this * rhs;
		return *this;
	}

	bool operator==(const Quaternion rhs) const
	{
		return std::fabs(x - rhs.x) < std::numeric_limits<float>::epsilon() &&
			std::fabs(y - rhs.y) < std::numeric_limits<float>::epsilon() &&
			std::fabs(z - rhs.z) < std::numeric_limits<float>::epsilon() &&
			std::fabs(w - rhs.w) < std::numeric_limits<float>::epsilon();
	}

	bool operator!=(const Quaternion rhs) const
	{
		return !(*this == rhs);
	}
};