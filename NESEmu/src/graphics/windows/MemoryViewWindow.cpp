#include "MemoryViewWindow.h"

#include <iomanip>

#include "util/Logger.h"
#include "util/Utils.h"

MemoryViewWindow::MemoryViewWindow(Bus &bus)
    : Window(GLFW_KEY_F2),
      currentPage(0),
      bus(bus)
{
}

void MemoryViewWindow::Draw()
{
    // If collapsed, exit out early as optimization
    if (!ImGui::Begin("Memory Viewer", &visible_))
    {
        ImGui::End();
        return;
    }

    ImGui::Text("CPU addressable memory:");
    ImGui::Spacing();

    // Initialize sub-window
    {
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_HorizontalScrollbar |
                                       ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::BeginChild("Memory",
                          ImVec2(ImGui::GetWindowContentRegionMax().x, 400),
                          true, windowFlags);

        // Display current memory page
        utils::PrintMemory(ss, pageSize * currentPage,
                           pageSize * (currentPage + 1),
                           [&](uint16_t address) { return bus.Read(address); });
        ImGui::Text(ss.str().c_str());
        ss.str("");

        ImGui::EndChild();
    }

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
    ss << std::hex << std::setw(4) << std::setfill('0')
       << pageSize * currentPage << " - " << std::setw(4) << std::setfill('0')
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

    // Dump entire memory region, 0x0000 - 0xFFFF
    if (ImGui::Button("Dump memory"))
    {
        Logger dump("..\\logs\\memdump.log");
        dump.Write("Memory dump\n\n");

        utils::PrintMemory(ss, 0, 0xFFFF,
                           [&](uint16_t address) { return bus.Read(address); });
        dump.Write(ss.str().c_str());

        printf("Dumped memory to logs/memdump.log");
        ss.str("");
    }

    ImGui::End();
}
