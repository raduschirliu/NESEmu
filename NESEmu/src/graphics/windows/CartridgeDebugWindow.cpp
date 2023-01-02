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

    Cartridge::Header header = cartridge_.GetHeader();
    IMapper *mapper = cartridge_.GetMapper();

    ImGui::Text("Mapper: %s (%u)", mapper->GetName().c_str(), mapper->GetId());
    ImGui::Text("Mirroring mode: %s",
                utils::MirroringModeToString(mapper->GetMirroringMode()));
    ImGui::Text("PRG ROM: %u banks (%u bytes)", header.prg_banks,
                header.prg_banks * Cartridge::PRG_BANK_SIZE);
    ImGui::Text("CHR ROM: %u banks (%u bytes)", header.chr_banks,
                header.chr_banks * Cartridge::CHR_BANK_SIZE);
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("ROM Header"))
    {
        ImGui::BeginChild("Cartridge##ROM Header", ImVec2(0, 0), true);

        ImGui::Text("Name: %s", header.name);
        ImGui::Text("PRG ROM Banks: %u", header.prg_banks);
        ImGui::Text("CHR ROM Banks: %u", header.chr_banks);

        ImGui::Text("Flags6");
        ImGui::Indent();
        ImGui::Text("Mirroring: %u", header.flags6.mirroring);
        ImGui::Text("Has PRG RAM: %u", header.flags6.has_prg_ram);
        ImGui::Text("Has Trainer: %u", header.flags6.has_trainer);
        ImGui::Text("Ignore Mirroring: %u", header.flags6.ignore_mirroring);
        ImGui::Text("Mapper Lower Nibble: $%X",
                    header.flags6.mapper_lower_nibble);
        ImGui::Unindent();

        ImGui::Text("Flags7");
        ImGui::Indent();
        ImGui::Text("VS Unisystem: %u", header.flags7.vs_unisystem);
        ImGui::Text("Has Playchoice Data: %u", header.flags7.has_playchoice_data);
        ImGui::Text("Format: $%X", header.flags7.format);
        ImGui::Text("Mapper Upper Nibble: $%X",
                    header.flags7.mapper_upper_nibble);
        ImGui::Unindent();

        ImGui::Text("PRG RAM Size: %u bytes", header.prg_ram_size);
        ImGui::Text("TV Flags 1: %s",
                    utils::ToBitString(header.tv_flags_1).c_str());
        ImGui::Text("TV Flags 2: %s",
                    utils::ToBitString(header.tv_flags_2).c_str());

        ImGui::EndChild();
    }
}