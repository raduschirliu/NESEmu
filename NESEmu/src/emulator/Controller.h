#pragma once

#include <bitset>
#include <cstdint>

#include "Bus.h"

class Controller
{
  public:
    enum class Button : uint8_t
    {
        kA = 0,
        kB,
        kSelect,
        kStart,
        kUp,
        kDown,
        kLeft,
        kRight,

        kCount
    };

    using ButtonStates = std::bitset<static_cast<size_t>(Button::kCount)>;

    Controller(Bus &bus, uint16_t port);

    ButtonStates GetButtonStates();
    void SetButtonStates(ButtonStates states);
    bool IsPolling();

  private:
    Bus &bus_;
    uint16_t output_register_;
    uint8_t current_index_;

    // Poll (strobe) current button states
    bool strobe_;
    ButtonStates button_states_;

    void OnBusMemoryAccess(uint16_t address, uint8_t new_value, bool write);
    void UpdateOutput();
};