#include "CartridgeDebugWindow.h"

CartridgeDebugWindow::CartridgeDebugWindow(Cartridge &CartridgeDebugWindow) : Window(GLFW_KEY_F4), cartridge(cartridge)
{
}

void CartridgeDebugWindow::draw()
{
	// If collapsed, exit out early as optimization
	if (!ImGui::Begin("Cartridge debugger", &visible))
	{
		ImGui::End();
		return;
	}
}
