#include "PPUDebugWindow.h"

#include <sstream>
#include <iomanip>
#include <vector>

bool createPatternTableTexture(PPU &ppu, uint8_t tableIndex, GLuint *texture);
static GLuint ptLeftTexture = 0;
static GLuint ptRightTexture = 0;

PPUDebugWindow::PPUDebugWindow(PPU &ppu) : Window(GLFW_KEY_F3), ppu(ppu)
{

}

void PPUDebugWindow::draw()
{
	// If window not enabled, don't draw it
	if (!enabled)
	{
		return;
	}

	// TODO: Make proper init routine
	if (ptRightTexture == 0)
	{
		// Create pattern table textures on first run
		IM_ASSERT(createPatternTableTexture(ppu, 0, &ptLeftTexture));
		IM_ASSERT(createPatternTableTexture(ppu, 1, &ptRightTexture));
	}

	// If collapsed, exit out early as optimization
	if (!ImGui::Begin("PPU Debugger", &enabled))
	{
		ImGui::End();
		return;
	}

	if (ImGui::Button("Dump CHR ROM"))
	{
		Logger dump("..\\logs\\chrrom_dump.log");
		dump.write("CHR ROM dump\n\n");

		printMemory(0x0000, 0x3FFF);
		dump.write(ss.str().c_str());

		printf("Dumped CHR ROM to logs");
		ss.str("");
	}

	ImGui::Text("Total cycles: %u\n", ppu.getTotalCycles());

	ImGui::Spacing();

	if (ImGui::CollapsingHeader("PPU Registers"))
	{
		PPU::Registers* registers = ppu.getRegisters();
		ImGui::Text("PPUCTRL ($2000): $%X\n", registers->ctrl);
		ImGui::Text("PPUMASK ($2001): $%X\n", registers->mask);
		ImGui::Text("PPUSTATUS ($2002): $%X\n", registers->status);
		ImGui::Text("OAMADDR ($2003): $%X\n", registers->oamAddr);
		ImGui::Text("OAMDATA ($2004): $%X\n", registers->oamData);
		ImGui::Text("PPUSCROLL ($2005): $%X\n", registers->scroll);
		ImGui::Text("PPUADDR ($2006): $%X\n", registers->addr);
		ImGui::Text("PPUDATA ($2007): $%X\n", registers->data);;
	}

	ImGui::Spacing();

	if (ImGui::BeginTabBar("PPU Views", ImGuiTabBarFlags_None))
	{
		// Pattern table
		if (ImGui::BeginTabItem("Pattern Table"))
		{
			PPU::Registers *registers = ppu.getRegisters();

			ImGui::Text("Background table: %d\n", (int)registers->ctrl.bgPatternTable);
			ImGui::Text("Sprite table: %d\n", (int)registers->ctrl.spritePatternTable);

			ImGui::Spacing();

			ImGui::Image((void *)(intptr_t)ptLeftTexture, ImVec2(256, 256));
			ImGui::SameLine();
			ImGui::Image((void*)(intptr_t)ptRightTexture, ImVec2(256, 256));

			ImGui::EndTabItem();
		}

		// Palette table
		if (ImGui::BeginTabItem("Palette Table"))
		{
			ImDrawList *drawList = ImGui::GetWindowDrawList();
			const ImVec2 pos = ImGui::GetCursorScreenPos();
			const float size = 64;
			const ImU32 textColor = ImColor(255, 255, 255);
			std::vector<PPU::Color> palette = ppu.getSystemPalette();

			for (int r = 0; r < 4; r++)
			{
				for (int c = 0; c < 16; c++)
				{
					float x = pos.x + c * size;
					float y = pos.y + r * size;
					int index = 16 * r + c;
					ImU32 color = ImColor(palette[index].r, palette[index].g, palette[index].b);
					drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + size, y + size), color);

					ss << "0x" << std::hex
						<< std::setw(2) << std::setfill('0')
						<< index;
					drawList->AddText(ImVec2(x, y), textColor, ss.str().c_str());
					ss.str("");
				}
			}

			ImGui::EndTabItem();
		}

		// Nametables
		if (ImGui::BeginTabItem("Nametable 1"))
		{
			drawNametable(0x2000);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Nametable 2"))
		{
			drawNametable(0x2400);
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}

void PPUDebugWindow::printMemory(uint16_t start, uint16_t end)
{
	// Draw 8 bytes per line from start address to end address
	for (int base = start; base <= end; base += 8)
	{
		ss << std::hex
			<< std::setw(4) << std::setfill('0')
			<< base << ":\t";

		for (int line = 0; line < 8; line++)
		{
			ss << std::setw(2) << std::setfill('0')
				<< (int)ppu.readMemory(base + line) << " ";
		}

		ss << std::endl;
	}
}

void PPUDebugWindow::drawNametable(uint16_t start)
{
	for (uint16_t r = 0; r < 30; r++)
	{
		for (uint16_t c = 0; c < 32; c++)
		{
			uint16_t offset = r * 32 + c;
			ss << std::hex
				<< std::setw(2) << std::setfill('0')
				<< (int)ppu.readMemory(start + offset)
				<< " ";
		}

		ss << std::endl;
	}
	
	ImGui::Text(ss.str().c_str());
	ss.str("");
}

bool createPatternTableTexture(PPU &ppu, uint8_t tableIndex, GLuint *texture)
{
	// Create OpenGL texture identifier
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	std::vector<uint8_t> pixels;
	uint8_t colors[] = { 0, 50, 100 };
	uint16_t base = (tableIndex & 0x01) << 12;

	// Create pixel data
	for (uint8_t r = 0; r < 128; r++)
	{
		for (uint8_t c = 0; c < 128; c++)
		{
			uint16_t patternIndex = 16 * (r / 8) + c / 8; // Which pattern index to use
			uint16_t byteIndex = r % 8; // Which byte of the pattern (row of pixels)
			uint8_t bitIndex = 7 - c % 8; // The bit (pixel) to use in the current byte

			uint16_t address = base | (patternIndex << 4) | byteIndex;
			uint8_t hiByte = ppu.readMemory(address);
			uint8_t loByte = ppu.readMemory(address + 8);
			uint8_t mask = 1 << bitIndex;
			uint8_t bit = ((hiByte & mask) >> (bitIndex - 1)) | ((loByte & mask) >> bitIndex);

			pixels.push_back(colors[bit]);
			pixels.push_back(colors[bit]);
			pixels.push_back(colors[bit]);
		}
	}

	// Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, &pixels[0]);

	return true;
}