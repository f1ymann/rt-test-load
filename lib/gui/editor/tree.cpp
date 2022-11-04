#include <tree.h>

rt3gui::DevicesTree::DevicesTree() {
    devices["SpawnInputActionNode"] = 1;
    devices["SpawnBranchNode"] = 2;
    devices["SpawnDoNNode"] = 3;
    devices["SpawnOutputActionNode"] = 4;
    devices["SpawnSetTimerNode"] = 5;
    devices["SpawnTreeSequenceNode"] = 6;
    devices["SpawnTreeTaskNode"] = 7;
    devices["SpawnTreeTask2Node"] = 8;
    devices["SpawnLessNode"] = 9;
    devices["SpawnWeirdNode"] = 10;
    devices["SpawnMessageNode"] = 11;
    devices["SpawnPrintStringNode"] = 12;  
    devices["SpawnHoudiniTransformNode"] = 13;
    devices["SpawnHoudiniGroupNode"] = 14;
    devices["SpawnTraceByChannelNode"] = 15; 
    devices["SpawnComment"] = 15;

}
void rt3gui::DevicesTree::DrawNodesTree() {

    ImGui::BeginGroup();

        if (ImGui::TreeNode("test devices")) {
            for (auto& device : devices) {
                    ImGui::PushID(device.first.c_str());
                    ImGui::Button(device.first.c_str(), ImVec2(120, 40));

                    // Our buttons are drag sources
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                    {
                        // Set payload to carry the index of our item (could be anything)
                        ImGui::SetDragDropPayload("TREE_DRAG_CELL", &device.second, sizeof(device.second));

                        // Display preview (could be anything, e.g. when dragging an image we could decide to display
                        // the filename and a small preview of the image, etc.)
                        ImGui::Text("Add device %s", device.first.c_str());
                        ImGui::EndDragDropSource();
                    }
                    ImGui::PopID();
                }

            ImGui::TreePop();
        }
    ImGui::EndGroup();
}

