#include "PPUDebugWindow.h"
#include "../Texture.h"

#include <sstream>
#include <iomanip>
#include <vector>

// TODO: Cleanup textures on de-init
static bool initialized = false;
static Texture ptLeftTexture(128, 128);
static Texture ptRightTexture(128, 128);

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

	// TODO: Make proper init method
	if (!initialized)
	{
		// Create pattern table textures on first run
		ptLeftTexture.load(ppu, 0);
		ptRightTexture.load(ppu, 0x1000);
		initialized = true;
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

			ImGui::Text("Background table: %d", (int)registers->ctrl.bgPatternTable);
			ImGui::Text("Sprite table: %d", (int)registers->ctrl.spritePatternTable);

			ImGui::Spacing();

			ImGui::Image((void *)(intptr_t)ptLeftTexture.getTextureId(), ImVec2(256, 256));
			ImGui::SameLine();
			ImGui::Image((void *)(intptr_t)ptRightTexture.getTextureId(), ImVec2(256, 256));

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

void PPUDebugWindow::drawNametable(uint16_t start)
{
	// Header
	ss << std::hex
		<< std::setw(2) << std::setfill('0')
		<< "    ";

	for (uint16_t c = 0; c < 32; c++)
	{
		ss << std::setw(2) << std::setfill('0') << (int)c << " ";
	}

	ImGui::Text(ss.str().c_str());
	ss.str("");
	
	// Nametable
	for (uint16_t r = 0; r < 30; r++)
	{
		ss << std::hex
			<< std::setw(2) << std::setfill('0')
			<< (int)r << "  ";

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
	
	// TEST: Drawing patterns from nametable indices
	float imgWidth = 256;
	float imgHeight = 240;

	for (uint16_t r = 0; r < 30; r++)
	{
		for (uint16_t c = 0; c < 32; c++)
		{
			uint16_t offset = r * 32 + c;
			uint8_t index = ppu.readMemory(start + offset);
			float rTex = index / 32;
			float cTex = index % 32;

			float xTex = cTex * 8;
			float yTex = rTex * 8;

			ImVec2 topLeft(xTex / imgWidth, yTex / imgHeight);
			ImVec2 botRight((xTex + 8) / imgWidth, (yTex + 8) / imgHeight);

			ImGui::Image((void *)(intptr_t)ptLeftTexture.getTextureId(), ImVec2(16, 16), topLeft, botRight);

			if (c != 31)
			{
				ImGui::SameLine();
			}
		}
	}
}