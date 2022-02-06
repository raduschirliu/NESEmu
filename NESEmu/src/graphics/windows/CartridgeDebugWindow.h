#pragma once

#include "../Window.h"
#include "../../emulator/Cartridge.h"

#include <sstream>

class CartridgeDebugWindow : public Window
{
public:
	CartridgeDebugWindow(Cartridge &cartridge);

	void draw() override;

private:
	Cartridge &cartridge;
	std::stringstream ss;
};
