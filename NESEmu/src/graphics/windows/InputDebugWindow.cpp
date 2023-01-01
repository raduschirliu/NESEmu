#include "InputDebugWindow.h"

InputDebugWindow::InputDebugWindow(Controller &controller)
    : Window(GLFW_KEY_F5),
      controller(controller)
{
}

void InputDebugWindow::Draw()
{
    // If collapsed, exit out early as optimization
    if (!ImGui::Begin("Input", &visible_))
    {
        ImGui::End();
        return;
    }

    // Input map
    ImGui::Text("Input map: joy1");
    ImGui::Text("Bitfield: %s", Input::GetKeyMap("joy1").to_string().c_str());
    ImGui::Spacing();

    // Controller
    ImGui::Text("Controller: joy1");
    ImGui::Text("Polling: %u", controller.isPolling());
    ImGui::Text("State:   %s",
                controller.getButtonStates().to_string().c_str());

    ImGui::End();
}
