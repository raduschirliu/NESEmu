#pragma once

#include <cstdint>
#include <vector>

// Represents a system palette color
struct Color
{
	uint8_t r, g, b, a;

	bool isTransparent() const { return a == 255; }
};

class Palette
{
public:
	Palette(size_t size = 0);
	Palette(std::vector<Color> colors);

	void setColors(const std::vector<Color> &colors);

	const size_t getSize() const;
	const std::vector<Color> &getColors() const;
	const std::vector<float> &getNormalized() const;

private:
	size_t size;
	std::vector<Color> colors;
	std::vector<float> normalized;
};