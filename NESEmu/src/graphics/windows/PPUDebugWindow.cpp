#include "PPUDebugWindow.h"

#include <sstream>
#include <iomanip>

void printMemory(PPU &ppu, std::stringstream &ss, uint16_t start, uint16_t end);

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

	// If collapsed, exit out early as optimization
	if (!ImGui::Begin("PPU Debugger", &enabled))
	{
		ImGui::End();
		return;
	}

	std::stringstream ss;

	if (ImGui::BeginTabBar("PPU Views", ImGuiTabBarFlags_None))
	{
		// Pattern table
		if (ImGui::BeginTabItem("Pattern Table"))
		{
			printMemory(ppu, ss, 0x0000, 0x0040);
			ImGui::Text(ss.str().c_str());
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}

void printMemory(PPU& ppu, std::stringstream& ss, uint16_t start, uint16_t end)
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
