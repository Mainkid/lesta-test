#pragma once
#include <math.h>
#include <iostream>
#include <limits>

#define EPS 0.00001f



struct Vector2
{
	float x;
	float y;

	void normalize()
	{
		auto length = sqrt(x * x + y * y);
		if (length < EPS)
			return;
		x /= length;
		y /= length;
	}

	Vector2 operator + (const Vector2& vec2) const
	{
		return Vector2{ x + vec2.x, y + vec2.y };
	}

	Vector2 operator - (const Vector2& vec2) const
	{
		return Vector2{ x - vec2.x, y - vec2.y };
	}

	Vector2 operator * (const float& rhs) const
	{
		return Vector2{ x * rhs,y * rhs };
	}

	Vector2& operator += (const Vector2& vec2)
	{
		x += vec2.x;
		y += vec2.y;
		return *this;
	}
	
};

struct Color
{
	/*
	Color bytes stored in ARGB order
	*/

	uint32_t value;

	Color()
	{
		value = std::numeric_limits<uint32_t>::max();				// White color
	}

	void setAlpha(float val)
	{
		value = (value & (~(0xFF<<24))) | (uint32_t(255*val)<<24);
	}

	float r()
	{
		return ((value >> 16) & 0xFF) / 255.0f;
	}

	float g()
	{
		return ((value >> 8) & 0xFF) / 255.0f;
	}

	float b()
	{
		return (value & 0xFF) / 255.0f;
	}

	float a()
	{
		return ((value >> 24) & 0xFF) / 255.0f;
	}


};

struct Particle
{
	Vector2 velocityVec;
	float lifeTime;
	float startLifeTime;
	float speed;
	bool isAlive = false;

	Particle() = default;

	
};

