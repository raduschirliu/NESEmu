#include "Controller.h"

using std::bind;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

Controller::Controller(Bus &bus, uint16_t port)
    : bus_(bus),
      output_register_(port),
      current_index_(0),
      strobe_(false)
{
    button_states_.reset();
    bus.RegisterMemoryAccessCallback(
        bind(&Controller::OnBusMemoryAccess, this, _1, _2, _3));
}

Controller::ButtonStates Controller::GetButtonStates()
{
    return button_states_;
}

void Controller::SetButtonStates(ButtonStates states)
{
    button_states_ = states;
}

bool Controller::IsPolling()
{
    return strobe_;
}

void Controller::OnBusMemoryAccess(uint16_t address, uint8_t new_value,
                                   bool write)
{
    if (address == Bus::JOY1 && write)
    {
        // We only care about bit 0
        strobe_ = (new_value & 0b1) == 1;

        // Start reading inputs in serial mode when strobe is disabled
        if (!strobe_)
        {
            current_index_ = 0;
            UpdateOutput();
            return;
        }
    }

    if (address == output_register_ && !write && !strobe_)
    {
        UpdateOutput();
    }
}

void Controller::UpdateOutput()
{
    if (current_index_ >= static_cast<uint8_t>(Button::kCount))
    {
        bus_.Write(output_register_, 0xFF, true);
        return;
    }

    uint8_t value = static_cast<uint8_t>(button_states_.test(current_index_));
    bus_.Write(output_register_, value, true);

    current_index_++;
}
