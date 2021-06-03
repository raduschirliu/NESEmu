#include "CpuWindow.h"

#include "Graphics.h"

CpuWindow::CpuWindow(CPU &cpu): cpu(cpu)
{
	enable();
}

void CpuWindow::draw()
{
	if (!enabled)
	{
		return;
	}

	ImGui::Begin("CPU", &enabled);

	ImGui::Text("Cycle: %d", cpu.getCycles());
	ImGui::Text("PC: $%X", cpu.getPC());
	ImGui::Text("Opcode: $%X", cpu.getOpcode());
	ImGui::Text("SP: $%X", cpu.getSP());

	char statusBuf[20];
	uint8_t statusRegister = cpu.getStatusRegister();

	for (int i = 0; i < 8; i++)
	{
		uint8_t bit = ((statusRegister >> 7) - i) & 0b00000001;
		sprintf_s(statusBuf + i, (size_t)20 - i, "%d", bit);
	}

	ImGui::Text("P: $%X | NO-BDIZC: %s", statusRegister, statusBuf);

	ImGui::Text("A: $%X", cpu.getA());
	ImGui::Text("X: $%X", cpu.getX());
	ImGui::Text("Y: $%X", cpu.getY());

	if (ImGui::Button("Step"))
	{
		cpu.step();
	}

	ImGui::End();
}