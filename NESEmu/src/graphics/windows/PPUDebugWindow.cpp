#include "PPUDebugWindow.h"
#include "../Texture.h"
#include "../ResourceManager.h"
#include "../../util/Utils.h"

#include <sstream>
#include <iomanip>
#include <vector>

using std::string;

// PPU Register help texts from NES wiki:
// https://wiki.nesdev.org/w/index.php/PPU_registers

static const char *CTRL_HELP_TEXT = R"(
7  bit  0
---- ----
VPHB SINN
|||| ||||
|||| ||++- Base nametable address
|||| ||    (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
|||| |+--- VRAM address increment per CPU read/write of PPUDATA
|||| |     (0: add 1, going across; 1: add 32, going down)
|||| +---- Sprite pattern table address for 8x8 sprites
||||       (0: $0000; 1: $1000; ignored in 8x16 mode)
|||+------ Background pattern table address (0: $0000; 1: $1000)
||+------- Sprite size (0: 8x8 pixels; 1: 8x16 pixels)
|+-------- PPU master/slave select
|          (0: read backdrop from EXT pins; 1: output color on EXT pins)
+--------- Generate an NMI at the start of the
           vertical blanking interval (0: off; 1: on)
)";

static const char *MASK_HELP_TEXT = R"(
7  bit  0
---- ----
BGRs bMmG
|||| ||||
|||| |||+- Greyscale (0: normal color, 1: produce a greyscale display)
|||| ||+-- 1: Show background in leftmost 8 pixels of screen, 0: Hide
|||| |+--- 1: Show sprites in leftmost 8 pixels of screen, 0: Hide
|||| +---- 1: Show background
|||+------ 1: Show sprites
||+------- Emphasize red (green on PAL/Dendy)
|+-------- Emphasize green (red on PAL/Dendy)
+--------- Emphasize blue
)";

static const char *STATUS_HELP_TEXT = R"(
7  bit  0
---- ----
VSO. ....
|||| ||||
|||+-++++- Least significant bits previously written into a PPU register
|||        (due to register not being updated for this address)
||+------- Sprite overflow. The intent was for this flag to be set whenever more than eight sprites appear on a scanline, but a
||         hardware bug causes the actual behavior to be more complicated and generate false positives as well as false negatives; see
||         PPU sprite evaluation. This flag is set during sprite evaluation and cleared at dot 1 (the second dot) of the pre-render line.
|+-------- Sprite 0 Hit.  Set when a nonzero pixel of sprite 0 overlaps a nonzero background pixel; cleared at dot 1 of the pre-render
|          line.  Used for raster timing.
+--------- Vertical blank has started (0: not in vblank; 1: in vblank). Set at dot 1 of line 241 (the line *after* the post-render
           line); cleared after reading $2002 and at dot 1 of the pre-render line.
)";

PPUDebugWindow::PPUDebugWindow(NES &nes, PPU &ppu) : Window(GLFW_KEY_F3), nes(nes), ppu(ppu), debugViewNametable(0)
{
	patternTableLeft = ResourceManager::getTexture("pattern_left");
	patternTableRight = ResourceManager::getTexture("pattern_right");
}

void PPUDebugWindow::draw()
{
	// If collapsed, exit out early as optimization
	if (!ImGui::Begin("PPU Debugger", &visible))
	{
		ImGui::End();
		return;
	}

	ImGui::Text("Current cycle: %u", ppu.getCycles());
	ImGui::Text("Current scanline: %u", ppu.getScanlines());
	ImGui::Text("Current frame: %u", ppu.getFrameCount());
	ImGui::Text("Total cycles: %u", ppu.getTotalCycles());

	ImGui::Dummy(ImVec2(0, 4));

	if (ImGui::BeginTabBar("PPU Views", ImGuiTabBarFlags_None))
	{
		// Registers
		if (ImGui::BeginTabItem("Registers"))
		{
			PPU::Registers *registers = ppu.getRegisters();
			drawRegister("PPUCTRL  ", 0x2000, &registers->ctrl, CTRL_HELP_TEXT);
			drawRegister("PPUMASK  ", 0x2001, &registers->mask, MASK_HELP_TEXT);
			drawRegister("PPUSTATUS", 0x2002, &registers->status, STATUS_HELP_TEXT);
			drawRegister("OAMADDR  ", 0x2003, &registers->oamAddr, nullptr);
			drawRegister("OAMDATA  ", 0x2004, &registers->oamData, nullptr);
			drawRegister("PPUSCROLL", 0x2005, &registers->scroll, nullptr);
			drawRegister("PPUADDR  ", 0x2006, &registers->addr, nullptr);
			drawRegister("PPUDATA  ", 0x2007, &registers->data, nullptr);

			ImGui::EndTabItem();
		}

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

			ImGui::Text("Display nametable: "); ImGui::SameLine();
			ImGui::RadioButton("1", &debugViewNametable, 0); ImGui::SameLine();
			ImGui::RadioButton("2", &debugViewNametable, 1); ImGui::SameLine();
			ImGui::RadioButton("3", &debugViewNametable, 2); ImGui::SameLine();
			ImGui::RadioButton("4", &debugViewNametable, 3);
			ImGui::Spacing();

			drawNametable(debugViewNametable);
			ImGui::EndTabItem();
		}

		// OAM
		if (ImGui::BeginTabItem("OAM"))
		{
			drawOam();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}

void PPUDebugWindow::drawRegister(string name, uint16_t address, const void* reg, const char* helpText)
{
	ImGui::Text(name.c_str());
	ImGui::SameLine();
	ImGui::Text(" ($%X):", address);
	ImGui::SameLine();

	const uint8_t *val = reinterpret_cast<const uint8_t*>(reg);
	ImGui::Text("  %s ($%X)", utils::toBitString(*val).c_str(), *val);

	if (helpText && ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("%s ($%X)", name.c_str(), address);
		ImGui::Text(CTRL_HELP_TEXT);
		ImGui::EndTooltip();
	}
}

void PPUDebugWindow::drawPalette(string label, std::vector<PPU::Color> palette)
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

	for (uint16_t r = 0; r < PPU::NAMETABLE_ROWS; r++)
	{
		for (uint16_t c = 0; c < PPU::NAMETABLE_COLS; c++)
		{
			uint16_t offset = r * PPU::NAMETABLE_COLS + c;
			uint8_t index = ppu.readMemory(address + offset);
			uint8_t paletteIndex = ppu.getNametableEntryPalette(nametable, offset);
			float cTex = index % PPU::PATTERN_TABLE_SIZE;
			float rTex = index / PPU::PATTERN_TABLE_SIZE;

			ImVec2 posTopLeft(cTex * PPU::TILE_SIZE, rTex * PPU::TILE_SIZE);
			ImVec2 posBottomRight(posTopLeft.x + PPU::TILE_SIZE, posTopLeft.y + PPU::TILE_SIZE);
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

			if (c != PPU::NAMETABLE_COLS - 1)
			{
				ImGui::SameLine();
			}
		}
	}
}

void PPUDebugWindow::drawOam()
{
	PPU::Registers *registers = ppu.getRegisters();
	ImGui::Text("Sprite table: %u ($%X)", registers->ctrl.spritePatternTable, ppu.getActiveSpritePatternTableAddress());
	ImGui::Spacing();

	for (int i = 0; i < PPU::OAM_SIZE; i++)
	{
		PPU::OamSprite *sprite = ppu.getOamSprite(i * sizeof(PPU::OamSprite));
		uint8_t *attributeByte = reinterpret_cast<uint8_t *>(&sprite->attributes);
		Texture *spriteTable = registers->ctrl.spritePatternTable == 0 ? patternTableLeft : patternTableRight;

		ImVec2 uvStart(
			std::floor(sprite->tileIndex % PPU::PATTERN_TABLE_SIZE * PPU::TILE_SIZE),
			std::floor(sprite->tileIndex / PPU::PATTERN_TABLE_SIZE * PPU::TILE_SIZE)
		);
		ImVec2 uvEnd(uvStart.x + PPU::TILE_SIZE, uvStart.y + PPU::TILE_SIZE);

		spriteTable->drawGui(ImVec2(32, 32), uvStart, uvEnd);

		if (ImGui::IsItemHovered())
		{
			float renderingScale = nes.getRenderingScale();
			float tileSize = nes.getTileSize();
			glm::vec2 offset = nes.getGraphicsOffset();
			glm::vec2 screenPos = glm::vec2(
				offset.x + sprite->xPos * renderingScale,
				offset.y + sprite->yPos * renderingScale
			);

			// TODO: Display attributes in more readable format
			ImGui::BeginTooltip();
			ImGui::Text("OAM Index:  %u", i);
			ImGui::Text("Pos:        %d, %d ($%X, $%X)", sprite->xPos, sprite->yPos, sprite->xPos, sprite->yPos);
			ImGui::Text("Pattern:    $%X", sprite->tileIndex);
			ImGui::Text("Attributes: $%X", *attributeByte);
			ImGui::Text("Screen Pos: %.2f, %.2f", screenPos.x, screenPos.y);
			ImGui::EndTooltip();
		}

		if ((i + 1) % 8 != 0)
		{
			ImGui::SameLine();
		}
	}
}
