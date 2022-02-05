#include "DebugWindow.h"

#include <algorithm>

DebugWindow::DebugWindow(NES &nes, CPU &cpu)
	: Window(GLFW_KEY_F1), prevTime(0), frames(0), fps(0), emulationSpeed(1.0), renderingScale(1), nes(nes), cpu(cpu)
{
	enable();
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

	// If window not enabled, don't draw it
	if (!enabled)
	{
		return;
	}

	// If collapsed, exit out early as optimization
	if (!ImGui::Begin("Debugger", &enabled))
	{
		ImGui::End();
		return;
	}

	// CPU state
	if (ImGui::CollapsingHeader("CPU State"))
	{
		CPU::State cpuState = cpu.getState();
		ImGui::Text("Cycle: %d", cpuState.totalCycles);
		ImGui::Text("PC: $%X", cpuState.pc);
		ImGui::Text("Opcode: $%X | %s (%s)", cpuState.opcode, cpuState.instruction.c_str(), cpuState.addressingMode.c_str());
		ImGui::Text("SP: $%X", cpuState.sp);

		char statusBuf[20];
		uint8_t statusRegister = cpuState.p;

		for (int i = 0; i < 8; i++)
		{
			uint8_t bit = ((statusRegister >> 7) - i) & 0b00000001;
			sprintf_s(statusBuf + i, (size_t)20 - i, "%d", bit);
		}

		ImGui::Text("P: $%X | NO-BDIZC: %s", statusRegister, statusBuf);

		ImGui::Text("A: $%X", cpuState.a);
		ImGui::Text("X: $%X", cpuState.x);
		ImGui::Text("Y: $%X", cpuState.y);
	}

	ImGui::Spacing();

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

	ImGui::End();
}