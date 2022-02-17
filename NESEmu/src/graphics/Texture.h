#pragma once

#include "Graphics.h"
#include "Shader.h"
#include "../emulator/PatternTable.h"
#include "../emulator/Palette.h"

#include <cstdint>
#include <vector>
#include <glm/glm.hpp>

class Texture
{
public:
	Texture(Shader *shader, int width, int height);
	Texture(const Texture &other);
	~Texture();

	void load(const PatternTable &patternTable);
	void update(const PatternTable &patternTable);
	void draw(glm::vec3 pos, glm::vec2 size, glm::vec2 uvTopLeft, glm::vec2 uvBottomRight,
		const Palette& palette, glm::vec4 color = glm::vec4(1.0f));

	void drawGui(ImVec2 size);
	void drawGui(ImVec2 size, ImVec2 texPosTopLeft, ImVec2 texPosBottomRight);

	GLuint getId();
	int getWidth();
	int getHeight();

private:
	Shader *shader;
	GLuint textureId;
	GLuint vaoId, vboId, eboId;
	int width, height;

	std::vector<uint8_t> getPixelData(const PatternTable &patternTable);
};