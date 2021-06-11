#include "MemoryViewWindow.h"

#include <iomanip>
#include <sstream>

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

	if (ImGui::Button("Dump memory"))
	{
		// Todo
	}

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_HorizontalScrollbar;
	ImGui::BeginChild("Memory", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 260), false, windowFlags);
	
	// Draw 8 bytes per line
	std::stringstream ss;
	static int page = 0;
	int pageSize = 0x00FF;

	for (int base = pageSize * page; base <= pageSize * (page + 1); base += 8)
	{
		ss << std::hex
			<< std::setw(4) << std::setfill('0')
			<< base << ":\t";

		for (int line = 0; line < 8; line++)
		{
			ss << std::setw(2) << std::setfill('0')
				<< (int)memory.read(base + line) << " ";
		}

		ss << std::endl;
	}

	ImGui::Text(ss.str().c_str());
	ss.str("");

	ImGui::EndChild();

	if (ImGui::Button("<<"))
	{
		if (page - 10 >= 0)
		{
			page -= 10;
		}
		else
		{
			page = 0;
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("<"))
	{
		if (page > 0)
		{
			page--;
		}
	}

	ImGui::SameLine();
	
	ss << std::hex
		<< std::setw(4) << std::setfill('0')
		<< pageSize * page
		<< " - "
		<< std::setw(4) << std::setfill('0')
		<< pageSize * (page + 1);
	ImGui::Text(ss.str().c_str());

	ImGui::SameLine();

	if (ImGui::Button(">"))
	{
		// TODO: Add check for upper bound
		page++;
	}

	ImGui::SameLine();

	if (ImGui::Button(">>"))
	{
		// TODO: Add check for upper bound
		page += 10;
	}

	ImGui::End();
}