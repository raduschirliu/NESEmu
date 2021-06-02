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
	ImGui::Text("P: $%X", cpu.getStatusRegister());
	ImGui::Text("A: $%X", cpu.getA());
	ImGui::Text("X: $%X", cpu.getX());
	ImGui::Text("Y: $%X", cpu.getY());

	if (ImGui::Button("Step"))
	{
		cpu.step();
	}

	ImGui::End();
}