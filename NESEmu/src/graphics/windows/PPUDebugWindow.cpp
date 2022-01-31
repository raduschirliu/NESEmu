#include "PPUDebugWindow.h"
#include "../Texture.h"
#include "../ResourceManager.h"

#include <sstream>
#include <iomanip>
#include <vector>

PPUDebugWindow::PPUDebugWindow(PPU &ppu) : Window(GLFW_KEY_F3), ppu(ppu)
{
	patternTableLeft = ResourceManager::getTexture("pattern_left");
	patternTableRight = ResourceManager::getTexture("pattern_right");
}

void PPUDebugWindow::draw()
{
	// If window not enabled, don't draw it
	if (!enabled)
	{
		return;
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

	ImGui::Text("Current cycle: %u", ppu.getCycles());
	ImGui::Text("Current scanline: %u", ppu.getScanlines());
	ImGui::Text("Current frame: %u", ppu.getFrameCount());
	ImGui::Text("Total cycles: %u", ppu.getTotalCycles());

	ImGui::Spacing();

	if (ImGui::CollapsingHeader("PPU Registers"))
	{
		PPU::Registers* registers = ppu.getRegisters();
		ImGui::Text("PPUCTRL  \t($2000): $%X", registers->ctrl);
		ImGui::Text("PPUMASK  \t($2001): $%X", registers->mask);
		ImGui::Text("PPUSTATUS\t($2002): $%X", registers->status);
		ImGui::Text("OAMADDR  \t($2003): $%X", registers->oamAddr);
		ImGui::Text("OAMDATA  \t($2004): $%X", registers->oamData);
		ImGui::Text("PPUSCROLL\t($2005): $%X", registers->scroll);
		ImGui::Text("PPUADDR  \t($2006): $%X", registers->addr);
		ImGui::Text("PPUDATA  \t($2007): $%X", registers->data);;
	}

	ImGui::Spacing();

	if (ImGui::BeginTabBar("PPU Views", ImGuiTabBarFlags_None))
	{
		// Pattern table
		if (ImGui::BeginTabItem("Pattern Table"))
		{
			PPU::Registers *registers = ppu.getRegisters();

			ImGui::Text("Background table:\t%u ($%X)", registers->ctrl.bgPatternTable, ppu.getActiveBgPatternTableAddress());
			ImGui::Text("Sprite table:\t\t%u ($%X)", registers->ctrl.spritePatternTable, ppu.getActiveSpritePatternTableAddress());

			ImGui::Spacing();

			patternTableLeft->drawGui(ImVec2(256, 256));
			ImGui::SameLine();
			patternTableRight->drawGui(ImVec2(256, 256));

			ImGui::EndTabItem();
		}

		// Palette table
		if (ImGui::BeginTabItem("Palette Table"))
		{
			drawPalette("System table", ppu.getSystemPalette());
			ImGui::Spacing();

			drawPalette("Background 0", ppu.getPalette(0x3F01)); // Background 0
			ImGui::SameLine();
			drawPalette("Background 1", ppu.getPalette(0x3F05)); // Background 1
			ImGui::SameLine();
			drawPalette("Background 2", ppu.getPalette(0x3F09)); // Background 2
			ImGui::SameLine();
			drawPalette("Background 3", ppu.getPalette(0x3F0D)); // Background 3

			drawPalette("Sprite 0", ppu.getPalette(0x3F11)); // Sprite 0
			ImGui::SameLine();
			drawPalette("Sprite 1", ppu.getPalette(0x3F15)); // Sprite 1
			ImGui::SameLine();
			drawPalette("Sprite 2", ppu.getPalette(0x3F19)); // Sprite 2
			ImGui::SameLine();
			drawPalette("Sprite 3", ppu.getPalette(0x3F1D)); // Sprite 3

			ImGui::EndTabItem();
		}

		// Nametables
		if (ImGui::BeginTabItem("Nametables"))
		{
			ImGui::Text("Active nametable: %u ($%X)", ppu.getRegisters()->ctrl.baseNametable, ppu.getActiveNametableAddress());
			ImGui::Spacing();

			static int debugNametable = 0;
			ImGui::Text("Display nametable: ");
			ImGui::RadioButton("1", &debugNametable, 0); ImGui::SameLine();
			ImGui::RadioButton("2", &debugNametable, 1); ImGui::SameLine();
			ImGui::RadioButton("3", &debugNametable, 2); ImGui::SameLine();
			ImGui::RadioButton("4", &debugNametable, 3);
			ImGui::Spacing();

			drawNametable(debugNametable);
			ImGui::EndTabItem();
		}

		// OAM
		if (ImGui::BeginTabItem("OAM"))
		{
			PPU::Registers *registers = ppu.getRegisters();
			ImGui::Text("Sprite table: %u ($%X)", registers->ctrl.spritePatternTable, ppu.getActiveSpritePatternTableAddress());
			ImGui::Spacing();

			for (int i = 0; i < 64; i++)
			{
				PPU::OamSprite *sprite = ppu.getOamSprite(i * sizeof(PPU::OamSprite));
				uint8_t *attributeByte = reinterpret_cast<uint8_t*>(&sprite->attributes);
				Texture *spriteTable = registers->ctrl.spritePatternTable == 0 ? patternTableLeft : patternTableRight;
				
				ImVec2 uvStart(
					std::floor(sprite->tileIndex % 16 * 8),
					std::floor(sprite->tileIndex / 16 * 8)
				);
				ImVec2 uvEnd(uvStart.x + 8, uvStart.y + 8);
				
				spriteTable->drawGui(ImVec2(32, 32), uvStart, uvEnd);

				if (ImGui::IsItemHovered())
				{
					// TODO: make these not hardcoded
					uint8_t renderingScale = 2;
					uint8_t tileSize = 8 * renderingScale;
					float offsetX = (1280 - tileSize * 32) / 2;
					float offsetY = (720 - tileSize * 30) / 2;
					float screenX = static_cast<float>(offsetX + sprite->xPos * renderingScale);
					float screenY = static_cast<float>(offsetY + sprite->yPos * renderingScale);

					// TODO: Display attributes in more readable format
					ImGui::BeginTooltip();
					ImGui::Text("OAM Index:  %u", i);
					ImGui::Text("Pos:        %d, %d ($%X, $%X)", sprite->xPos, sprite->yPos, sprite->xPos, sprite->yPos);
					ImGui::Text("Pattern:    $%X", sprite->tileIndex);
					ImGui::Text("Attributes: $%X", *attributeByte);
					ImGui::Text("Screen Pos: %.2f, %.2f", screenX, screenY);
					ImGui::EndTooltip();
				}

				if ((i + 1) % 8 != 0)
				{
					ImGui::SameLine();
				}
			}

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

void PPUDebugWindow::drawPalette(std::string label, std::vector<PPU::Color> palette)
{
	ImGui::BeginGroup();
	ImGui::Text(label.c_str());

	ImDrawList *drawList = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	int cols = 0;
	int rows = 1;
	int maxCols = 0;
	int index = 0;
	const float size = 64;
	const ImU32 textColor = ImColor(255, 255, 255);
	const float xStart = pos.x;
	ImVec2 totalSize;

	for (PPU::Color color : palette)
	{
		if (cols >= 16)
		{
			cols = 0;
			rows++;
			pos.x = xStart;
			pos.y += size;
		}

		ImU32 imColor = ImColor(color.r, color.g, color.b);
		drawList->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + size, pos.y + size), imColor);

		ss << "0x" << std::hex
			<< std::setw(2) << std::setfill('0')
			<< index;
		drawList->AddText(ImVec2(pos.x, pos.y), textColor, ss.str().c_str());
		ss.str("");

		cols++;
		maxCols = std::max(cols, maxCols);
		pos.x += size;
		index++;
	}

	totalSize.x = maxCols * size + 10;
	totalSize.y = rows * size + 10;
	ImGui::Dummy(totalSize);
	ImGui::EndGroup();
}

void PPUDebugWindow::drawNametable(uint8_t nametable)
{
	uint16_t address = PPU::NAMETABLE_ADDRESSES[nametable];
	ImVec2 tileSize(16, 16);
	PPU::Registers *registers = ppu.getRegisters();
	Texture *bgTable = registers->ctrl.bgPatternTable == 0 ? patternTableLeft : patternTableRight;

	for (uint16_t r = 0; r < 30; r++)
	{
		for (uint16_t c = 0; c < 32; c++)
		{
			uint16_t offset = r * 32 + c;
			uint8_t index = ppu.readMemory(address + offset);
			uint8_t paletteIndex = ppu.getNametableEntryPalette(nametable, offset);
			float cTex = index % 16;
			float rTex = index / 16;

			ImVec2 posTopLeft(cTex * 8, rTex * 8);
			ImVec2 posBottomRight(posTopLeft.x + 8, posTopLeft.y + 8);
			bgTable->drawGui(tileSize, posTopLeft, posBottomRight);

			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text("Pos:             %d, %d ($%X, $%X)", r, c, r, c);
				ImGui::Text("Nametable Index: %u ($X)", offset, offset);
				ImGui::Text("Pattern Index:   %u ($X)", index, index);
				ImGui::Text("Palette Index:   %u", paletteIndex);
				ImGui::EndTooltip();
			}

			if (c != 31)
			{
				ImGui::SameLine();
			}
		}
	}
}