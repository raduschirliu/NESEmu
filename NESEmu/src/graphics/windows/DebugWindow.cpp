#include "DebugWindow.h"

DebugWindow::DebugWindow(NES &nes, CPU &cpu): Window(GLFW_KEY_F1), nes(nes), cpu(cpu)
{
	enable();
}

void DebugWindow::draw()
{
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

	CPU::State cpuState = cpu.getState();

	{
		ImGui::BeginGroup();
		ImGui::Text("Cycle: %d", cpuState.totalCycles);
		ImGui::Text("PC: $%X", cpuState.pc);
		ImGui::Text("Opcode: $%X", cpuState.opcode);
		ImGui::Text("SP: $%X", cpuState.sp);
		ImGui::EndGroup();
	}

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

	// Step & play/pause buttons
	{
		ImGui::BeginGroup();
		if (ImGui::Button("Step"))
		{
			nes.step();
		}

		ImGui::SameLine();

		if (ImGui::Button(nes.getRunning() ? "Pause" : "Play"))
		{
			nes.setRunning(!nes.getRunning());
		}
		ImGui::EndGroup();
	}

	ImGui::End();
}