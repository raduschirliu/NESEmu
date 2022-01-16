#pragma once

struct Vec2
{
	Vec2() : x(0), y(0) {}
	Vec2(float x, float y) : x(x), y(y) {}

	Vec2 operator+(const Vec2 &rhs) { return Vec2(x + rhs.x, y + rhs.y); }

	float x, y;
};