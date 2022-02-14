#include "Palette.h"

#include <assert.h>

Palette::Palette(size_t size) : size(size), colors({}), normalized({})
{

}

Palette::Palette(std::vector<Color> colors) : size(colors.size())
{
	setColors(colors);
}

void Palette::setColors(const std::vector<Color> &colors)
{
	if (size == 0)
	{
		size = colors.size();
	}

	assert(size == colors.size());

	this->colors = colors;
	normalized.clear();
	normalized.reserve(colors.size() * 4);

	for (auto &color : colors)
	{
		normalized.push_back(color.r / 255.0f);
		normalized.push_back(color.g / 255.0f);
		normalized.push_back(color.b / 255.0f);
		normalized.push_back(color.a / 255.0f);
	}
}

const size_t Palette::getSize() const
{
	return size;
}

const std::vector<Color> &Palette::getColors() const
{
	return colors;
}

const std::vector<float> &Palette::getNormalized() const
{
	return normalized;
}