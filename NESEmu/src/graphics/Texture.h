#pragma once

#include "Graphics.h"
#include "Vec2.h"
#include "../emulator/PPU.h"

#include <cstdint>
#include <vector>

class Texture
{
public:
	Texture(int width, int height);
	~Texture();

	void load(PPU& ppu, uint16_t baseAddress);
	void update(PPU &ppu, uint16_t baseAddress);
	void draw(Vec2 pos, Vec2 size, Vec2 uvTopLeft, Vec2 uvBottomRight);

	GLuint getTextureId();

private:
	GLuint textureId;
	int width, height;

	// TODO: Implement grayscale texture
	bool grayscale;

	std::vector<uint8_t> getPixelData(PPU &ppu, uint16_t baseAddress);
};