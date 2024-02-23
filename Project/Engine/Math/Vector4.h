#pragma once
struct Vector4
{
	float x;
	float y;
	float z;
	float w;

	Vector4 operator+(const Vector4& rhs) const
	{
		Vector4 result{};
		result.x = x + rhs.x;
		result.y = y + rhs.y;
		result.z = z + rhs.z;
		result.w = w + rhs.w;
		return result;
	}

	Vector4 operator-(const Vector4& rhs) const
	{
		Vector4 result{};
		result.x = x - rhs.x;
		result.y = y - rhs.y;
		result.z = z - rhs.z;
		result.w = w - rhs.w;
		return result;
	}

	Vector4 operator*(const Vector4& rhs) const
	{
		Vector4 result{};
		result.x = x * rhs.x;
		result.y = y * rhs.y;
		result.z = z * rhs.z;
		result.w = w * rhs.w;
		return result;
	}

	Vector4 operator*(float scalar) const
	{
		Vector4 result{};
		result.x = x * scalar;
		result.y = y * scalar;
		result.z = z * scalar;
		result.w = w * scalar;
		return result;
	}

	Vector4 operator/(float scalar) const
	{
		Vector4 result{};
		if (scalar != 0.0f)
		{
			result.x = x / scalar;
			result.y = y / scalar;
			result.z = z / scalar;
			result.w = w / scalar;
		}
		return result;
	}

	Vector4 operator+=(const Vector4& rhs)
	{
		*this = *this + rhs;
		return *this;
	}

	Vector4 operator-=(const Vector4& rhs)
	{
		*this = *this - rhs;
		return *this;
	}

	Vector4 operator*=(const Vector4& rhs)
	{
		*this = *this * rhs;
		return *this;
	}

	Vector4 operator*=(float scalar)
	{
		*this = *this * scalar;
		return *this;
	}

	Vector4 operator/=(float scalar)
	{
		*this = *this / scalar;
		return *this;
	}

	bool operator==(const Vector4& rhs) const
	{
		return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
	}

	bool operator!=(const Vector4& rhs) const
	{
		return !(*this == rhs);
	}
};