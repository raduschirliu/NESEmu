#include "Texture.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using std::vector;

Texture::Texture(Shader *shader, int width, int height) : 
	shader(shader), textureId(0), vaoId(0), vboId(0), eboId(0), width(width), height(height)
{

}

Texture::Texture(const Texture &other) :
	shader(other.shader), textureId(other.textureId), vaoId(other.vaoId),
	vboId(other.vboId), eboId(other.eboId), width(other.width), height(other.height)
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

void Texture::load(const PatternTable &patternTable)
{
	// Create and bind buffers
	glGenBuffers(1, &vboId);
	glGenBuffers(1, &eboId);
	glGenVertexArrays(1, &vaoId);

	assert(vboId != 0);
	assert(eboId != 0);
	assert(vaoId != 0);
	GL_ERROR_CHECK();

	glBindVertexArray(vaoId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId);
	glBindVertexArray(0);

	GL_ERROR_CHECK();

	vector<uint8_t> pixels = getPixelData(patternTable);

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

	GL_ERROR_CHECK();
}

void Texture::update(const PatternTable &patternTable)
{
	vector<uint8_t> pixels = getPixelData(patternTable);

	glBindTexture(GL_TEXTURE_2D, textureId);
	// Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, &pixels[0]);
	glBindTexture(GL_TEXTURE_2D, 0);

	GL_ERROR_CHECK();
}

/*
void Texture::draw(glm::vec2 pos, glm::vec2 size)
{
	vector<PPU::Color> palette(4);
	draw(glm::vec3(pos, 0.0f), size, glm::vec2(0.0f), glm::vec2(width, height), palette, glm::vec4(1.0f));
}
*/

void Texture::draw(glm::vec3 pos, glm::vec2 size, glm::vec2 uvTopLeft, glm::vec2 uvBottomRight,
	const Palette& palette, glm::vec4 color)
{
	assert(palette.size() == 4);

	// Enable shader and set uniforms
	shader->use();

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, pos);
	model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));
	// TODO: Update projection matrix when window is changed
	glm::mat4 projection = glm::ortho<float>(0.0f, 1280.0f, 720.0f, 0.0f, -10.0f, 10.0f);

	shader->setVector4f("colorModifier", color);
	shader->setVector4f("palette", palette.getNormalized());
	shader->setMatrix4f("model", model);
	shader->setMatrix4f("projection", projection);

	// Enable texture alpha, depth test
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glBindVertexArray(vaoId);

	// Update VAO
	// For UVs: bottom left = (0, 0), top right = (1.0, 1.0)
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

	// Update EBO
	unsigned int indices[] =
	{
		0, 1, 3, // First triangle
		1, 2, 3 // Second triangle
	};

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Draw vertices
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	// Unbind, disable, and cleanup
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	shader->abandon();

	GL_ERROR_CHECK();
}

void Texture::drawGui(ImVec2 size)
{
	drawGui(size, ImVec2(0.0f, 0.0f), ImVec2(width, height));
}

void Texture::drawGui(ImVec2 size, ImVec2 texPosTopLeft, ImVec2 texPosBottomRight)
{
	// For ImGui: Top left = (0.0, 0.0), bottom right = (1.0, 1.0)
	ImVec2 uvStart(texPosTopLeft.x / width, texPosTopLeft.y / height);
	ImVec2 uvEnd(texPosBottomRight.x / width, texPosBottomRight.y / height);

	ImGui::Image((void *)(intptr_t)textureId, size, uvStart, uvEnd);
}

GLuint Texture::getId()
{
	return textureId;
}

int Texture::getWidth()
{
	return width;
}

int Texture::getHeight()
{
	return height;
}

vector<uint8_t> Texture::getPixelData(const PatternTable &patternTable)
{
	vector<uint8_t> pixels;
	pixels.reserve(128 * 128 * 3);

	uint8_t colors[] = { 0, 51, 102, 153 }; // { 0.0, 0.2, 0.4, 0.6 } * 255
	const size_t pixelSize = PatternTable::SIZE * PatternTable::TILE_SIZE;

	// Loop through all pixels of the texture and find the corresponding pattern to get the pixel data from
	for (size_t y = 0; y < pixelSize; y++)
	{
		for (size_t x = 0; x < pixelSize; x++)
		{
			// Cordinates of the pattern tile in the pattern table
			size_t patternRow = y / PatternTable::TILE_SIZE;
			size_t patternCol = x / PatternTable::TILE_SIZE;

			// Cordinates of the pixel within the pattern
			size_t patternY = y % PatternTable::TILE_SIZE;
			size_t patternX = x % PatternTable::TILE_SIZE;

			size_t pixelIndex = patternY * PatternTable::TILE_SIZE + patternX;
			uint8_t color = patternTable.getPattern(patternRow, patternCol)[pixelIndex];

			pixels.push_back(colors[color]);
			pixels.push_back(colors[color]);
			pixels.push_back(colors[color]);
		}
	}

	return pixels;
}