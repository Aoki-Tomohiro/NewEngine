#pragma once
struct Vector2 
{
	float x;
	float y;

	Vector2 operator+(const Vector2& rhs) const
	{
		Vector2 result{};
		result.x = x + rhs.x;
		result.y = y + rhs.y;
		return result;
	}

	Vector2 operator-(const Vector2& rhs) const
	{
		Vector2 result{};
		result.x = x - rhs.x;
		result.y = y - rhs.y;
		return result;
	}

	Vector2 operator*(const Vector2& rhs) const
	{
		Vector2 result{};
		result.x = x * rhs.x;
		result.y = y * rhs.y;
		return result;
	}

	Vector2 operator*(float scalar) const
	{
		Vector2 result{};
		result.x = x * scalar;
		result.y = y * scalar;
		return result;
	}

	Vector2 operator/(float scalar) const
	{
		Vector2 result{};
		if (scalar != 0.0f)
		{
			result.x = x / scalar;
			result.y = y / scalar;
		}
		return result;
	}

	Vector2 operator+=(const Vector2& rhs)
	{
		*this = *this + rhs;
		return *this;
	}

	Vector2 operator-=(const Vector2& rhs)
	{
		*this = *this - rhs;
		return *this;
	}

	Vector2 operator*=(const Vector2& rhs)
	{
		*this = *this * rhs;
		return *this;
	}

	Vector2 operator*=(float scalar)
	{
		*this = *this * scalar;
		return *this;
	}

	Vector2 operator/=(float scalar)
	{
		*this = *this / scalar;
		return *this;
	}

	bool operator==(const Vector2& rhs) const
	{
		return x == rhs.x && y == rhs.y;
	}

	bool operator!=(const Vector2& rhs) const
	{
		return !(*this == rhs);
	}
};