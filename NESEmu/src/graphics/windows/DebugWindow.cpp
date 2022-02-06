#include "DebugWindow.h"
#include "../../util/Utils.h"

#include <algorithm>

// Help texts from NES Wiki:
// https://wiki.nesdev.org/w/index.php/Status_flags
static const char *STATUS_HELP_TEXT = R"(
7  bit  0
---- ----
NVss DIZC
|||| ||||
|||| |||+- Carry
|||| ||+-- Zero
|||| |+--- Interrupt Disable
|||| +---- Decimal
||++------ No CPU effect, see: the B flag
|+-------- Overflow
+--------- Negative
)";

DebugWindow::DebugWindow(NES &nes, CPU &cpu)
	: Window(GLFW_KEY_F1), prevTime(0), frames(0), fps(0), emulationSpeed(1.0), renderingScale(1), nes(nes), cpu(cpu)
{
	setVisible(true);
}

void DebugWindow::draw()
{
	// Measure FPS
	double currentTime = glfwGetTime();
	frames++;

	if (currentTime - prevTime >= 1.0)
	{
		fps = frames;
		frames = 0;
		prevTime = currentTime;
	}

	// If collapsed, exit out early as optimization
	if (!ImGui::Begin("Debugger", &visible))
	{
		ImGui::End();
		return;
	}

	// Emulator controls
	{
		ImGui::BeginGroup();
		ImGui::Text("FPS: %u", fps);
		ImGui::Spacing();

		if (ImGui::Button("Step"))
		{
			nes.step();
		}

		ImGui::SameLine();

		if (ImGui::Button(nes.getRunning() ? "Pause" : "Play"))
		{
			nes.setRunning(!nes.getRunning());
		}

		if (ImGui::InputDouble("Emulation speed", &emulationSpeed))
		{
			emulationSpeed = std::max(0.0, std::min(emulationSpeed, 5.0));
			nes.setEmulationSpeed(emulationSpeed);
		}

		const char *comboLabels[] = { "1x", "2x", "4x", "8x" };
		if (ImGui::BeginCombo("Rendering scale", comboLabels[renderingScale]))
		{
			for (int n = 0; n < IM_ARRAYSIZE(comboLabels); n++)
			{
				const bool isSelected = (renderingScale == n);
				if (ImGui::Selectable(comboLabels[n], isSelected))
				{
					renderingScale = n;
					float scale = std::powf(2, renderingScale);
					nes.setRenderingScale(scale);
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}

		ImGui::EndGroup();
	}

	// CPU state
	CPU::State cpuState = cpu.getState();
	ImGui::Text("Cycle:   %d", cpuState.totalCycles);
	ImGui::Text("PC:      $%X", cpuState.pc);
	ImGui::Text("Opcode:  $%X | %s (%s)", cpuState.opcode, cpuState.instruction.c_str(), cpuState.addressingMode.c_str());
	ImGui::Text("SP:      $%X", cpuState.sp);

	ImGui::Text("P:       $%X (%s)", cpuState.p, utils::toBitString(cpuState.p).c_str());

	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip(STATUS_HELP_TEXT);
	}

	ImGui::Text("A:       $%X", cpuState.a);
	ImGui::Text("X:       $%X", cpuState.x);
	ImGui::Text("Y:       $%X", cpuState.y);

	ImGui::Spacing();

	ImGui::End();
}