#include "Controller.h"

using std::bind;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

Controller::Controller(Bus &bus, uint16_t port) : bus(bus), outputRegister(port), currentIndex(0), strobe(false)
{
	buttonStates.reset();
	bus.registerMemoryAccessCallback(bind(&Controller::onBusMemoryAccess, this, _1, _2, _3));
}

Controller::ButtonStates Controller::getButtonStates()
{
	return buttonStates;
}

void Controller::setButtonStates(ButtonStates states)
{
	buttonStates = states;
}

bool Controller::isPolling()
{
	return strobe;
}

void Controller::onBusMemoryAccess(uint16_t address, uint8_t newValue, bool write)
{
	if (address == Bus::JOY1 && write)
	{
		// We only care about bit 0
		strobe = (newValue & 0b1) == 1;

		// Start reading inputs in serial mode when strobe is disabled
		if (!strobe)
		{
			currentIndex = 0;
			updateOutput();
			return;
		}
	}

	if (address == outputRegister && !write && !strobe)
	{
		updateOutput();
	}
}

void Controller::updateOutput()
{
	if (currentIndex >= static_cast<uint8_t>(Button::COUNT))
	{
		bus.write(outputRegister, 0xFF, true);
		return;
	}

	uint8_t value = static_cast<uint8_t>(buttonStates.test(currentIndex));
	bus.write(outputRegister, value, true);

	currentIndex++;
}
