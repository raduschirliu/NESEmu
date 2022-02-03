#pragma once

#include "../Window.h"
#include "../../util/Input.h"
#include "../../emulator/Controller.h"

class InputDebugWindow : public Window
{
public:
	InputDebugWindow(Controller &controller);

	void draw() override;

private:
	Controller &controller;
};