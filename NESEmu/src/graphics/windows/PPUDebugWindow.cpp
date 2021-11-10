#include "PPUDebugWindow.h"

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

	ImGui::End();
}