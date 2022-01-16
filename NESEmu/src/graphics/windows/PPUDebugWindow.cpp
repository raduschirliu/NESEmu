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
		ImGui::Text("PPUCTRL\t($2000): $%X", registers->ctrl);
		ImGui::Text("PPUMASK\t($2001): $%X", registers->mask);
		ImGui::Text("PPUSTATUS\t($2002): $%X", registers->status);
		ImGui::Text("OAMADDR\t($2003): $%X", registers->oamAddr);
		ImGui::Text("OAMDATA\t($2004): $%X", registers->oamData);
		ImGui::Text("PPUSCROLL\t($2005): $%X", registers->scroll);
		ImGui::Text("PPUADDR\t($2006): $%X", registers->addr);
		ImGui::Text("PPUDATA\t($2007): $%X", registers->data);;
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