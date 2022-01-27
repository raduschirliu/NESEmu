#include "Texture.h"
#include <iostream>

Texture::Texture(int width, int height) : textureId(0), vaoId(0), vboId(0), eboId(0), width(width), height(height)
{

}

Texture::~Texture()
{
	if (textureId != 0)
	{
		glDeleteBuffers(1, &eboId);
		glDeleteBuffers(1, &vboId);
		glDeleteVertexArrays(1, &vaoId);
		glDeleteTextures(1, &textureId);
	}
}

void Texture::load(PPU &ppu, uint16_t baseAddress)
{
	// Create and bind buffers
	glGenBuffers(1, &vboId);
	glGenBuffers(1, &eboId);
	glGenVertexArrays(1, &vaoId);

	glBindVertexArray(vaoId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId);
	glBindVertexArray(0);

	std::vector<uint8_t> pixels = getPixelData(ppu, baseAddress);

	// Create OpenGL texture identifier
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);

	// Setup filtering parameters for display (using nearest for pixelated textures)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, &pixels[0]);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::update(PPU &ppu, uint16_t baseAddress)
{
	std::vector<uint8_t> pixels = getPixelData(ppu, baseAddress);

	glBindTexture(GL_TEXTURE_2D, textureId);
	// Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, &pixels[0]);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::draw(Vec2 pos, Vec2 size)
{
	draw(pos, size, Vec2(0, 0), Vec2(width, height));
}

void Texture::draw(Vec2 pos, Vec2 size, Vec2 uvTopLeft, Vec2 uvBottomRight)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glBindVertexArray(vaoId);

	float vertices[] =
	{
		// Position				// Texture coords
		1.0f, 1.0f, 0.0f,		uvBottomRight.x / width,	uvBottomRight.y / height,	// top right
		1.0f, 0.0f, 0.0f,		uvBottomRight.x / width,	uvTopLeft.y / height,		// bottom right
		0.0f, 0.0f, 0.0f,		uvTopLeft.x / width,		uvTopLeft.y / height,		// bottom left
		0.0f, 1.0f, 0.0f,		uvTopLeft.x / width,		uvBottomRight.y / height,	// top left
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	unsigned int indices[] =
	{
		0, 1, 3, // First triangle
		1, 2, 3 // Second triangle
	};

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint Texture::getTextureId()
{
	return textureId;
}

std::vector<uint8_t> Texture::getPixelData(PPU &ppu, uint16_t baseAddress)
{
	std::vector<uint8_t> pixels;
	uint8_t colors[] = { 0, 51, 102, 153 }; // { 0.0, 0.2, 0.4, 0.6 } * 255

	for (uint8_t r = 0; r < 128; r++)
	{
		for (uint8_t c = 0; c < 128; c++)
		{
			uint16_t patternIndex = 16 * (r / 8) + c / 8; // Which pattern index to use
			uint16_t byteIndex = r % 8; // Which byte of the pattern (row of pixels)
			uint8_t bitIndex = 7 - c % 8; // The bit (pixel) to use in the current byte

			uint16_t address = baseAddress | (patternIndex << 4) | byteIndex;
			uint8_t loByte = ppu.readMemory(address);
			uint8_t hiByte = ppu.readMemory(address + 8);
			uint8_t mask = 1 << bitIndex;
			uint8_t bit = ((hiByte & mask) >> (bitIndex - 1)) | ((loByte & mask) >> bitIndex);

			pixels.push_back(colors[bit]);
			pixels.push_back(colors[bit]);
			pixels.push_back(colors[bit]);
		}
	}

	return pixels;
}