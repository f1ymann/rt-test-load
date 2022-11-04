# include "widgets.h"
# define IMGUI_DEFINE_MATH_OPERATORS
# include <imgui_internal.h>

void ax::Widgets::Icon(const ImVec2& size, IconType type, bool filled, const ImVec4& color/* = ImVec4(1, 1, 1, 1)*/, const ImVec4& innerColor/* = ImVec4(0, 0, 0, 0)*/, bool leftTriangle)
{
    if (ImGui::IsRectVisible(size))
    {
        auto cursorPos = ImGui::GetCursorScreenPos();
        auto drawList  = ImGui::GetWindowDrawList();
        ax::Drawing::DrawIcon(drawList, cursorPos, cursorPos + size, type, filled, ImColor(color), ImColor(innerColor), leftTriangle);
    }

    ImGui::Dummy(size);
}

void ax::Widgets::Util_BeginColumnGroup(float width)
{
    // Push dummy widget to extend node size. Columns do not do that.
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    ImGui::Dummy(ImVec2(width, 0));
    ImGui::PopStyleVar();

    // Start columns, but use only first one.
    ImGui::BeginColumns("##TreeColumns", 2,
        ImGuiColumnsFlags_NoBorder |
        ImGuiColumnsFlags_NoResize |
        ImGuiColumnsFlags_NoPreserveWidths |
        ImGuiColumnsFlags_NoForceWithinWindow);

    // Adjust column width to match requested one.
    ImGui::SetColumnWidth(0, width
        + ImGui::GetStyle().WindowPadding.x
        + ImGui::GetStyle().ItemSpacing.x);
}

void ax::Widgets::Util_EndColumnGroup()
{
    ImGui::EndColumns();
}
