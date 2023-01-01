#include "CartridgeDebugWindow.h"

#include "util/Utils.h"

CartridgeDebugWindow::CartridgeDebugWindow(Cartridge &cartridge)
    : Window(GLFW_KEY_F4),
      cartridge_(cartridge)
{
}

void CartridgeDebugWindow::Draw()
{
    // If collapsed, exit out early as optimization
    if (!ImGui::Begin("Cartridge debugger", &visible_))
    {
        ImGui::End();
        return;
    }

    Cartridge::Header header = cartridge_.getHeader();
    IMapper *mapper = cartridge_.getMapper();

    ImGui::Text("Mapper: %s (%u)", mapper->GetName().c_str(), mapper->GetId());
    ImGui::Text("Mirroring mode: %s",
                utils::MirroringModeToString(mapper->GetMirroringMode()));
    ImGui::Text("PRG ROM: %u banks (%u bytes)", header.prgBanks,
                header.prgBanks * Cartridge::PRG_BANK_SIZE);
    ImGui::Text("CHR ROM: %u banks (%u bytes)", header.chrBanks,
                header.chrBanks * Cartridge::CHR_BANK_SIZE);
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("ROM Header"))
    {
        ImGui::BeginChild("Cartridge##ROM Header", ImVec2(0, 0), true);

        ImGui::Text("Name: %s", header.name);
        ImGui::Text("PRG ROM Banks: %u", header.prgBanks);
        ImGui::Text("CHR ROM Banks: %u", header.chrBanks);

        ImGui::Text("Flags6");
        ImGui::Indent();
        ImGui::Text("Mirroring: %u", header.flags6.mirroring);
        ImGui::Text("Has PRG RAM: %u", header.flags6.hasPrgRam);
        ImGui::Text("Has Trainer: %u", header.flags6.hasTrainer);
        ImGui::Text("Ignore Mirroring: %u", header.flags6.ignoreMirroring);
        ImGui::Text("Mapper Lower Nibble: $%X",
                    header.flags6.mapperLowerNibble);
        ImGui::Unindent();

        ImGui::Text("Flags7");
        ImGui::Indent();
        ImGui::Text("VS Unisystem: %u", header.flags7.vsUnisystem);
        ImGui::Text("Has Playchoice Data: %u", header.flags7.hasPlaychoiceData);
        ImGui::Text("Format: $%X", header.flags7.format);
        ImGui::Text("Mapper Upper Nibble: $%X",
                    header.flags7.mapperUpperNibble);
        ImGui::Unindent();

        ImGui::Text("PRG RAM Size: %u bytes", header.prgRamSize);
        ImGui::Text("TV Flags 1: %s",
                    utils::ToBitString(header.tvFlags1).c_str());
        ImGui::Text("TV Flags 2: %s",
                    utils::ToBitString(header.tvFlags2).c_str());

        ImGui::EndChild();
    }
}