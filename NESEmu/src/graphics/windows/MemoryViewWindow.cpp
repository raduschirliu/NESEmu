#include "MemoryViewWindow.h"
#include "../../debug/Logger.h"

#include <iomanip>

MemoryViewWindow::MemoryViewWindow(Memory &memory) : Window(GLFW_KEY_F2), currentPage(0), memory(memory)
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
		// Dump entire memory region, 0x0000 - 0xFFFF
		Logger dump("..\\logs\\memdump.log");
		dump.write("Memory dump\n\n");

		printMemory(0, 0xFFFF);
		dump.write(ss.str().c_str());
		
		printf("Dumped memory to logs/memdump.log");
		ss.str("");
	}

	// Initialize window
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_HorizontalScrollbar;
	ImGui::BeginChild("Memory", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 260), false, windowFlags);

	// Display current memory page
	printMemory(pageSize * currentPage, pageSize * (currentPage + 1));
	ImGui::Text(ss.str().c_str());
	ss.str("");

	ImGui::EndChild();

	// Jump 10 pages back
	if (ImGui::Button("<<"))
	{
		if (currentPage - 10 >= 0)
		{
			currentPage -= 10;
		}
		else
		{
			currentPage = 0;
		}
	}

	ImGui::SameLine();

	// Previous page
	if (ImGui::Button("<"))
	{
		if (currentPage > 0)
		{
			currentPage--;
		}
	}

	ImGui::SameLine();

	// Current page range
	ss << std::hex
		<< std::setw(4) << std::setfill('0')
		<< pageSize * currentPage
		<< " - "
		<< std::setw(4) << std::setfill('0')
		<< pageSize * (currentPage + 1);
	ImGui::Text(ss.str().c_str());
	ss.str("");

	ImGui::SameLine();

	// Calculate the memory range that the page shows up until
	int pageStart = currentPage * pageSize;

	// Next page
	if (ImGui::Button(">"))
	{
		if (pageStart + pageSize < 0xFFFF)
		{
			currentPage++;
		}
	}

	ImGui::SameLine();

	// Jump 10 pages forward
	if (ImGui::Button(">>"))
	{
		if (pageStart + pageSize * 10 < 0xFFFF)
		{
			currentPage += 10;
		}
		else
		{
			currentPage = (0xFFFF - pageSize) / pageSize;
		}
	}

	ImGui::End();
}

void MemoryViewWindow::printMemory(uint16_t start, uint16_t end)
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
				<< (int)memory.read(base + line) << " ";
		}

		ss << std::endl;
	}
}
