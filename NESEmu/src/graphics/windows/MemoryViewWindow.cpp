#include "MemoryViewWindow.h"

MemoryViewWindow::MemoryViewWindow(Memory &memory): Window(GLFW_KEY_F2), memory(memory)
{

}

void MemoryViewWindow::draw()
{
	// If window not enabled, don't draw it
	if (!enabled)
	{
		return;
	}

	// If collapsed, exit out early as optimization
	if (!ImGui::Begin("Memory Viewer", &enabled))
	{
		ImGui::End();
		return;
	}



	ImGui::End();
}