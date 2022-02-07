#pragma once

#include "Bus.h"

#include <cstdint>
#include <bitset>

class Controller
{
public:
	enum class Button : uint8_t
	{
		A = 0,
		B,
		SELECT,
		START,
		UP,
		DOWN,
		LEFT,
		RIGHT,

		COUNT
	};

	using ButtonStates = std::bitset<static_cast<size_t>(Button::COUNT)>;

	Controller(Bus &bus, uint16_t port);
	ButtonStates getButtonStates();
	void setButtonStates(ButtonStates states);
	bool isPolling();

private:
	Bus &bus;
	uint16_t outputRegister;
	uint8_t currentIndex;

	// Poll (strobe) current button states
	bool strobe;
	ButtonStates buttonStates;

	void onBusMemoryAccess(uint16_t address, uint8_t newValue, bool write);
	void updateOutput();
};