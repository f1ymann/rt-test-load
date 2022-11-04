#include <editor.h>
#include <schemeEditor.h>

uint32_t rt3gui::IdGenerator::GenerateId()
{
    return m_State++;
}

void rt3gui::IdGenerator::SetState(uint32_t state)
{
    m_State = state;
}

uint32_t rt3gui::IdGenerator::State() const
{
    return m_State;
}


rt3gui::Editor::Editor(const char* _id, std::shared_ptr < TexturesLoader> _textureLoader, ed::Config& config, Document* _document) {
    g_FirstFrame = true;     
    g_NextLinkId = 100;
    m_Editor = nullptr;
    s_HeaderBackground = nullptr;
    document = _document;
    m_Generator.SetState(1);
    s_TouchTime = 1.0f;

    id = _id;
    id.append("#Editor");

    textureLoader = _textureLoader;
    visible = true;


    m_Editor = ed::CreateEditor(&config);
    ed::SetCurrentEditor(m_Editor);

    //CreateExampleDocument();



    createNewNode = false;
    newNodeLinkPin = nullptr;
    newLinkPin = nullptr;
    contextNodeId = 0;
    contextLinkId = 0;
    contextPinId = 0;

    
    Node* node;


    //ed::NavigateToContent();

    BuildNodes();
#ifdef _WIN32
    s_HeaderBackground = textureLoader->Application_LoadTexture("Data\\BlueprintBackground.png");
#else
    s_HeaderBackground = textureLoader->Application_LoadTexture("Data/BlueprintBackground.png");
#endif // _WIN32



}


rt3gui::Editor::~Editor() {
    if (m_Editor)
    {
        ed::DestroyEditor(m_Editor);
        m_Editor = nullptr;
    }
}

ed::LinkId rt3gui::Editor::GetNextLinkId()
{
    return ed::LinkId(m_Generator.GenerateId());
}

void rt3gui::Editor::TouchNode(ed::NodeId id)
{
    s_NodeTouchTime[id] = s_TouchTime;
}

float rt3gui::Editor::GetTouchProgress(ed::NodeId id)
{
    auto it = s_NodeTouchTime.find(id);
    if (it != s_NodeTouchTime.end() && it->second > 0.0f)
        return (s_TouchTime - it->second) / s_TouchTime;
    else
        return 0.0f;
}

void rt3gui::Editor::UpdateTouch()
{
    const auto deltaTime = ImGui::GetIO().DeltaTime;
    for (auto& entry : s_NodeTouchTime)
    {
        if (entry.second > 0.0f)
            entry.second -= deltaTime;
    }
}

const rt3gui::Node* rt3gui::Editor::FindNode(ed::NodeId id) const
{
    for (auto& node : s_Nodes)
        if (node.ID == id)
            return &node;

    return nullptr;
}

rt3gui::Link* rt3gui::Editor::FindLink(ed::LinkId id)
{
    for (auto& link : s_Links)
        if (link->ID == id)
            return link.get();

    return nullptr;
}

rt3gui::Pin* rt3gui::Editor::FindPin(ed::PinId id)
{
    if (!id)
        return nullptr;

    for (auto& node : s_Nodes)
    {
        for (auto& pin : node.Inputs)
            if (pin.ID == id)
                return &pin;

        for (auto& pin : node.Outputs)
            if (pin.ID == id)
                return &pin;
    }

    return nullptr;
}

bool rt3gui::Editor::IsPinLinked(ed::PinId id)
{
    if (!id)
        return false;

    for (auto& link : s_Links)
        if (link->StartPinID == id || link->EndPinID == id)
            return true;

    return false;
}

bool rt3gui::Editor::CanCreateLink(Pin* a, Pin* b)
{
    if (!a || !b || a == b || a->Kind == b->Kind || a->Type != b->Type || a->Node == b->Node || (bool(a->Kind)&&IsPinLinked(a->ID)) || (bool(b->Kind) && IsPinLinked(b->ID)))
        return false;

    return true;
}

void rt3gui::Editor::BuildNode(Node* node)
{
    for (auto& input : node->Inputs)
    {
        input.Node = node;
        input.Kind = PinKind::Input;
    }

    for (auto& output : node->Outputs)
    {
        output.Node = node;
        output.Kind = PinKind::Output;
    }
}


void rt3gui::Editor::BuildNodes()
{
    for (auto& node : s_Nodes)
        BuildNode(&node);
}

ImColor rt3gui::Editor::GetIconColor(rt3gui::SampleTypes type)
{
    switch (type)
    {
    default:
    case rt3gui::SampleTypes::ComplexFloat: return ImColor(220, 48, 48);
    case rt3gui::SampleTypes::FloatPoint:   return ImColor(147, 226, 74);
    case rt3gui::SampleTypes::Integer:      return ImColor(255, 255, 255);
    }
};

void rt3gui::Editor::DrawPinIcon(const Pin& pin, bool connected, int alpha)
{
    IconType iconType;
    ImColor  color = GetIconColor(pin.Type);
    color.Value.w = alpha / 255.0f;
    switch (pin.Type)
    {
    case  rt3gui::SampleTypes::ComplexFloat: iconType = IconType::Circle; break;
    case  rt3gui::SampleTypes::FloatPoint:   iconType = IconType::Circle; break;
    case  rt3gui::SampleTypes::Integer:      iconType = IconType::Circle; break;
    default:
        return;
    }

    ax::Widgets::Icon(ImVec2(s_PinIconSize, s_PinIconSize), iconType, connected, color, ImColor(32, 32, 32, alpha), bool(pin.Kind));
};

static inline ImRect ImGui_GetItemRect()
{
    return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
}

static inline ImRect ImRect_Expanded(const ImRect& rect, float x, float y)
{
    auto result = rect;
    result.Min.x -= x;
    result.Min.y -= y;
    result.Max.x += x;
    result.Max.y += y;
    return result;
}


void rt3gui::Editor::drawEditor() {
    ed::SetCurrentEditor(m_Editor);

    if (visible) {

        UpdateTouch();

        ed::Begin(id.c_str());
        {
            auto cursorTopLeft = ImGui::GetCursorScreenPos();

            util::BlueprintNodeBuilder builder(s_HeaderBackground, textureLoader->Application_GetTextureWidth(s_HeaderBackground), textureLoader->Application_GetTextureHeight(s_HeaderBackground));

            for (auto& node : s_Nodes)
            {
                if (node.Type != NodeType::Blueprint && node.Type != NodeType::Simple)
                    continue;

                const auto isSimple = node.Type == NodeType::Simple;

                if (restoreNodes) {
                    ed::RestoreNodeState(node.ID); //  ///!!!///
                }

                builder.Begin(node.ID);
                if (!isSimple)
                {
                    builder.Header(node.Color);
                    ImGui::Spring(0);
                    ImGui::TextUnformatted(node.Name.c_str());
                    ImGui::Spring(1);
                    ImGui::Dummy(ImVec2(0, 28));
                    ImGui::Spring(0);
                    builder.EndHeader();
                }

                for (auto& input : node.Inputs)
                {
                    auto alpha = ImGui::GetStyle().Alpha;
                    if (newLinkPin && !CanCreateLink(newLinkPin, &input) && &input != newLinkPin)
                        alpha = alpha * (48.0f / 255.0f);

                    builder.Input(input.ID);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
                    DrawPinIcon(input, IsPinLinked(input.ID), (int)(alpha * 255));
                    ImGui::Spring(0);
                    if (!input.Name.empty())
                    {
                        ImGui::TextUnformatted(input.Name.c_str());
                        ImGui::Spring(0);
                    }

                    ImGui::PopStyleVar();
                    builder.EndInput();
                }


                if (isSimple)
                {
                    builder.Middle();

                    ImGui::Spring(1, 0);
                    ImGui::TextUnformatted(node.Name.c_str());
                    ImGui::Spring(1, 0);
                }

                for (auto& output : node.Outputs)
                {

                    auto alpha = ImGui::GetStyle().Alpha;
                    if (newLinkPin && !CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
                        alpha = alpha * (48.0f / 255.0f);

                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
                    builder.Output(output.ID);
                    if (!output.Name.empty())
                    {
                        ImGui::Spring(0);
                        ImGui::TextUnformatted(output.Name.c_str());
                    }
                    ImGui::Spring(0);
                    DrawPinIcon(output, IsPinLinked(output.ID), (int)(alpha * 255));
                    ImGui::PopStyleVar();
                    builder.EndOutput();
                }

                builder.End();
            }

            for (auto& node : s_Nodes)
            {
                if (node.Type != NodeType::Tree)
                    continue;

                const float rounding = 5.0f;
                const float padding = 12.0f;

                const auto pinBackground = ed::GetStyle().Colors[ed::StyleColor_NodeBg];

                ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(128, 128, 128, 200));
                ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(32, 32, 32, 200));
                ed::PushStyleColor(ed::StyleColor_PinRect, ImColor(60, 180, 255, 150));
                ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor(60, 180, 255, 150));

                ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(0, 0, 0, 0));
                ed::PushStyleVar(ed::StyleVar_NodeRounding, rounding);
                ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.0f, 1.0f));
                ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
                ed::PushStyleVar(ed::StyleVar_LinkStrength, 0.0f);
                ed::PushStyleVar(ed::StyleVar_PinBorderWidth, 1.0f);
                ed::PushStyleVar(ed::StyleVar_PinRadius, 5.0f);
                ed::BeginNode(node.ID);

                ImGui::BeginVertical(node.ID.AsPointer());
                ImGui::BeginHorizontal("inputs");
                ImGui::Spring(0, padding * 2);

                ImRect inputsRect;
                int inputAlpha = 200;
                if (!node.Inputs.empty())
                {
                    auto& pin = node.Inputs[0];
                    ImGui::Dummy(ImVec2(0, padding));
                    ImGui::Spring(1, 0);
                    inputsRect = ImGui_GetItemRect();

                    ed::PushStyleVar(ed::StyleVar_PinArrowSize, 10.0f);
                    ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 10.0f);
                    ed::PushStyleVar(ed::StyleVar_PinCorners, 12);
                    ed::BeginPin(pin.ID, ed::PinKind::Input);
                    ed::PinPivotRect(inputsRect.GetTL(), inputsRect.GetBR());
                    ed::PinRect(inputsRect.GetTL(), inputsRect.GetBR());
                    ed::EndPin();
                    ed::PopStyleVar(3);

                    if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
                        inputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
                }
                else
                    ImGui::Dummy(ImVec2(0, padding));

                ImGui::Spring(0, padding * 2);
                ImGui::EndHorizontal();

                ImGui::BeginHorizontal("content_frame");
                ImGui::Spring(1, padding);

                ImGui::BeginVertical("content", ImVec2(0.0f, 0.0f));
                ImGui::Dummy(ImVec2(160, 0));
                ImGui::Spring(1);
                ImGui::TextUnformatted(node.Name.c_str());
                ImGui::Spring(1);
                ImGui::EndVertical();
                auto contentRect = ImGui_GetItemRect();

                ImGui::Spring(1, padding);
                ImGui::EndHorizontal();

                ImGui::BeginHorizontal("outputs");
                ImGui::Spring(0, padding * 2);

                ImRect outputsRect;
                int outputAlpha = 200;
                if (!node.Outputs.empty())
                {
                    auto& pin = node.Outputs[0];
                    ImGui::Dummy(ImVec2(0, padding));
                    ImGui::Spring(1, 0);
                    outputsRect = ImGui_GetItemRect();

                    ed::PushStyleVar(ed::StyleVar_PinCorners, 3);
                    ed::BeginPin(pin.ID, ed::PinKind::Output);
                    ed::PinPivotRect(outputsRect.GetTL(), outputsRect.GetBR());
                    ed::PinRect(outputsRect.GetTL(), outputsRect.GetBR());
                    ed::EndPin();
                    ed::PopStyleVar();

                    if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
                        outputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
                }
                else
                    ImGui::Dummy(ImVec2(0, padding));

                ImGui::Spring(0, padding * 2);
                ImGui::EndHorizontal();

                ImGui::EndVertical();

                ed::EndNode();
                ed::PopStyleVar(7);
                ed::PopStyleColor(4);

                auto drawList = ed::GetNodeBackgroundDrawList(node.ID);

                //const auto fringeScale = ImGui::GetStyle().AntiAliasFringeScale;
                //const auto unitSize    = 1.0f / fringeScale;

                //const auto ImDrawList_AddRect = [](ImDrawList* drawList, const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners, float thickness)
                //{
                //    if ((col >> 24) == 0)
                //        return;
                //    drawList->PathRect(a, b, rounding, rounding_corners);
                //    drawList->PathStroke(col, true, thickness);
                //};

                drawList->AddRectFilled(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
                    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
                //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
                drawList->AddRect(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
                    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
                //ImGui::PopStyleVar();
                drawList->AddRectFilled(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
                    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
                //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
                drawList->AddRect(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
                    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
                //ImGui::PopStyleVar();
                drawList->AddRectFilled(contentRect.GetTL(), contentRect.GetBR(), IM_COL32(24, 64, 128, 200), 0.0f);
                //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
                drawList->AddRect(
                    contentRect.GetTL(),
                    contentRect.GetBR(),
                    IM_COL32(48, 128, 255, 100), 0.0f);
                //ImGui::PopStyleVar();
            }

            for (auto& node : s_Nodes)
            {
                if (node.Type != NodeType::Houdini)
                    continue;

                const float rounding = 10.0f;
                const float padding = 12.0f;


                ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(229, 229, 229, 200));
                ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(125, 125, 125, 200));
                ed::PushStyleColor(ed::StyleColor_PinRect, ImColor(229, 229, 229, 60));
                ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor(125, 125, 125, 60));

                const auto pinBackground = ed::GetStyle().Colors[ed::StyleColor_NodeBg];

                ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(0, 0, 0, 0));
                ed::PushStyleVar(ed::StyleVar_NodeRounding, rounding);
                ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.0f, 1.0f));
                ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
                ed::PushStyleVar(ed::StyleVar_LinkStrength, 0.0f);
                ed::PushStyleVar(ed::StyleVar_PinBorderWidth, 1.0f);
                ed::PushStyleVar(ed::StyleVar_PinRadius, 6.0f);
                ed::BeginNode(node.ID);

                ImGui::BeginVertical(node.ID.AsPointer());
                if (!node.Inputs.empty())
                {
                    ImGui::BeginHorizontal("inputs");
                    ImGui::Spring(1, 0);

                    ImRect inputsRect;
                    int inputAlpha = 200;
                    for (auto& pin : node.Inputs)
                    {
                        ImGui::Dummy(ImVec2(padding, padding));
                        inputsRect = ImGui_GetItemRect();
                        ImGui::Spring(1, 0);
                        inputsRect.Min.y -= padding;
                        inputsRect.Max.y -= padding;

                        //ed::PushStyleVar(ed::StyleVar_PinArrowSize, 10.0f);
                        //ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 10.0f);
                        ed::PushStyleVar(ed::StyleVar_PinCorners, 15);
                        ed::BeginPin(pin.ID, ed::PinKind::Input);
                        ed::PinPivotRect(inputsRect.GetCenter(), inputsRect.GetCenter());
                        ed::PinRect(inputsRect.GetTL(), inputsRect.GetBR());
                        ed::EndPin();
                        //ed::PopStyleVar(3);
                        ed::PopStyleVar(1);

                        auto drawList = ImGui::GetWindowDrawList();
                        drawList->AddRectFilled(inputsRect.GetTL(), inputsRect.GetBR(),
                            IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 15);
                        drawList->AddRect(inputsRect.GetTL(), inputsRect.GetBR(),
                            IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 15);

                        if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
                            inputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
                    }

                    //ImGui::Spring(1, 0);
                    ImGui::EndHorizontal();
                }

                ImGui::BeginHorizontal("content_frame");
                ImGui::Spring(1, padding);

                ImGui::BeginVertical("content", ImVec2(0.0f, 0.0f));
                ImGui::Dummy(ImVec2(160, 0));
                ImGui::Spring(1);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                ImGui::TextUnformatted(node.Name.c_str());
                ImGui::PopStyleColor();
                ImGui::Spring(1);
                ImGui::EndVertical();
                auto contentRect = ImGui_GetItemRect();

                ImGui::Spring(1, padding);
                ImGui::EndHorizontal();

                if (!node.Outputs.empty())
                {
                    ImGui::BeginHorizontal("outputs");
                    ImGui::Spring(1, 0);

                    ImRect outputsRect;
                    int outputAlpha = 200;
                    for (auto& pin : node.Outputs)
                    {
                        ImGui::Dummy(ImVec2(padding, padding));
                        outputsRect = ImGui_GetItemRect();
                        ImGui::Spring(1, 0);
                        outputsRect.Min.y += padding;
                        outputsRect.Max.y += padding;

                        ed::PushStyleVar(ed::StyleVar_PinCorners, 3);
                        ed::BeginPin(pin.ID, ed::PinKind::Output);
                        ed::PinPivotRect(outputsRect.GetCenter(), outputsRect.GetCenter());
                        ed::PinRect(outputsRect.GetTL(), outputsRect.GetBR());
                        ed::EndPin();
                        ed::PopStyleVar();

                        auto drawList = ImGui::GetWindowDrawList();
                        drawList->AddRectFilled(outputsRect.GetTL(), outputsRect.GetBR(),
                            IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 15);
                        drawList->AddRect(outputsRect.GetTL(), outputsRect.GetBR(),
                            IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 15);


                        if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
                            outputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
                    }

                    ImGui::EndHorizontal();
                }

                ImGui::EndVertical();

                ed::EndNode();
                ed::PopStyleVar(7);
                ed::PopStyleColor(4);

                auto drawList = ed::GetNodeBackgroundDrawList(node.ID);

                //const auto fringeScale = ImGui::GetStyle().AntiAliasFringeScale;
                //const auto unitSize    = 1.0f / fringeScale;

                //const auto ImDrawList_AddRect = [](ImDrawList* drawList, const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners, float thickness)
                //{
                //    if ((col >> 24) == 0)
                //        return;
                //    drawList->PathRect(a, b, rounding, rounding_corners);
                //    drawList->PathStroke(col, true, thickness);
                //};

                //drawList->AddRectFilled(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
                //    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
                //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
                //drawList->AddRect(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
                //    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
                //ImGui::PopStyleVar();
                //drawList->AddRectFilled(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
                //    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
                ////ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
                //drawList->AddRect(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
                //    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
                ////ImGui::PopStyleVar();
                //drawList->AddRectFilled(contentRect.GetTL(), contentRect.GetBR(), IM_COL32(24, 64, 128, 200), 0.0f);
                //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
                //drawList->AddRect(
                //    contentRect.GetTL(),
                //    contentRect.GetBR(),
                //    IM_COL32(48, 128, 255, 100), 0.0f);
                //ImGui::PopStyleVar();
            }

            for (auto& node : s_Nodes)
            {
                if (node.Type != NodeType::Comment)
                    continue;

                const float commentAlpha = 0.75f;

                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha);
                ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(255, 255, 255, 64));
                ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(255, 255, 255, 64));
                ed::BeginNode(node.ID);
                ImGui::PushID(node.ID.AsPointer());
                ImGui::BeginVertical("content");
                ImGui::BeginHorizontal("horizontal");
                ImGui::Spring(1);
                ImGui::TextUnformatted(node.Name.c_str());
                ImGui::Spring(1);
                ImGui::EndHorizontal();
                ed::Group(node.Size);
                ImGui::EndVertical();
                ImGui::PopID();
                ed::EndNode();
                ed::PopStyleColor(2);
                ImGui::PopStyleVar();

                if (ed::BeginGroupHint(node.ID))
                {
                    //auto alpha   = static_cast<int>(commentAlpha * ImGui::GetStyle().Alpha * 255);
                    auto bgAlpha = static_cast<int>(ImGui::GetStyle().Alpha * 255);

                    //ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha * ImGui::GetStyle().Alpha);

                    auto min = ed::GetGroupMin();
                    //auto max = ed::GetGroupMax();

                    ImGui::SetCursorScreenPos(min - ImVec2(-8, ImGui::GetTextLineHeightWithSpacing() + 4));
                    ImGui::BeginGroup();
                    ImGui::TextUnformatted(node.Name.c_str());
                    ImGui::EndGroup();

                    auto drawList = ed::GetHintBackgroundDrawList();

                    auto hintBounds = ImGui_GetItemRect();
                    auto hintFrameBounds = ImRect_Expanded(hintBounds, 8, 4);

                    drawList->AddRectFilled(
                        hintFrameBounds.GetTL(),
                        hintFrameBounds.GetBR(),
                        IM_COL32(255, 255, 255, 64 * bgAlpha / 255), 4.0f);

                    drawList->AddRect(
                        hintFrameBounds.GetTL(),
                        hintFrameBounds.GetBR(),
                        IM_COL32(255, 255, 255, 128 * bgAlpha / 255), 4.0f);

                    //ImGui::PopStyleVar();
                }
                ed::EndGroupHint();
            }


            for (auto& link : s_Links)
                ed::Link(link->ID, link->StartPinID, link->EndPinID, link->Color, 2.0f);




            if (!createNewNode)
            {
                if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f))
                {
                    auto showLabel = [](const char* label, ImColor color)
                    {
                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
                        auto size = ImGui::CalcTextSize(label);

                        auto padding = ImGui::GetStyle().FramePadding;
                        auto spacing = ImGui::GetStyle().ItemSpacing;

                        ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

                        auto rectMin = ImGui::GetCursorScreenPos() - padding;
                        auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

                        auto drawList = ImGui::GetWindowDrawList();
                        drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
                        ImGui::TextUnformatted(label);
                    };

                    ed::PinId startPinId = 0, endPinId = 0;
                    if (ed::QueryNewLink(&startPinId, &endPinId))
                    {
                        auto startPin = FindPin(startPinId);
                        auto endPin = FindPin(endPinId);

                        newLinkPin = startPin ? startPin : endPin;

                        if (startPin->Kind == PinKind::Input)
                        {
                            std::swap(startPin, endPin);
                            std::swap(startPinId, endPinId);
                        }

                        if (startPin && endPin)
                        {
                            if (endPin == startPin)
                            {
                                ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                            }
                            else if (endPin->Kind == startPin->Kind)
                            {
                                showLabel("x Incompatible Pin Kind", ImColor(45, 32, 32, 180));
                                ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                            }
                            else if (endPin->Node == startPin->Node)
                            {
                                showLabel("x Cannot connect to self", ImColor(45, 32, 32, 180));
                                ed::RejectNewItem(ImColor(255, 0, 0), 1.0f);
                            }
                            else if (endPin->Type != startPin->Type)
                            {
                                showLabel("x Incompatible Pin Type", ImColor(45, 32, 32, 180));
                                ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
                            }
                            else if (bool(endPin->Kind) && IsPinLinked(endPin->ID)) {
                                showLabel("x Input Pin Already Connected", ImColor(45, 32, 32, 180));
                                ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
                            }
                            else
                            {
                                showLabel("+ Create Link", ImColor(32, 45, 32, 180));
                                if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
                                {
                                    auto transaction = document->BeginUndoTransaction("Create Link"); //коммит нового линка                                       
                                    s_Links.push_back(std::unique_ptr<Link>(new Link(m_Generator.GenerateId(), startPinId, endPinId)));
                                    s_Links.back()->Color = GetIconColor(startPin->Type);
                                    LOGV("[HandleCreateAction] %" PRI_pin " linked with %" PRI_pin, FMT_pin(startPin), FMT_pin(endPin));
                                    transaction->AddAction("% " PRI_pin " linked with % " PRI_pin, FMT_pin(startPin), FMT_pin(endPin));
                                }
                            }
                        }
                    }

                    ed::PinId pinId = 0;
                    if (ed::QueryNewNode(&pinId))
                    {
                        newLinkPin = FindPin(pinId);
                        if (newLinkPin)
                            showLabel("+ Create Node", ImColor(32, 45, 32, 180));

                        if (ed::AcceptNewItem())
                        {
                            createNewNode = true;
                            newNodeLinkPin = FindPin(pinId);
                            newLinkPin = nullptr;
                            ed::Suspend();
                            ImGui::OpenPopup("Create New Node");
                            ed::Resume();
                        }
                    }
                }
                else
                    newLinkPin = nullptr;

                ed::EndCreate();

                if (ed::BeginDelete())
                {
                    auto deferredTransaction = document->GetDeferredUndoTransaction("Destroy Action"); //коммит удаления нод и линков

                    ed::LinkId linkId = 0;
                    while (ed::QueryDeletedLink(&linkId))
                    {
                        deferredTransaction->Begin("Delete Item");

                        if (ed::AcceptDeletedItem())
                        {
                            auto id = std::find_if(s_Links.begin(), s_Links.end(), [linkId](auto& link) { return link->ID == linkId; });
                            if (id != s_Links.end()) {
                                Pin* startPin = FindPin(id.operator->()->get()->StartPinID);
                                Pin* endPin = FindPin(id.operator->()->get()->EndPinID);
                                s_Links.erase(id);
                                LOGV("[HandleDestroyAction] %" PRI_pin " unlinked from %" PRI_pin, FMT_pin(startPin), FMT_pin(endPin));
                            }

                        }
                    }

                    ed::NodeId nodeId = 0;
                    while (ed::QueryDeletedNode(&nodeId))
                    {
                        deferredTransaction->Begin("Delete Item");

                        ImVec2 pos = ed::GetNodePosition(nodeId);
                        auto id = std::find_if(s_Nodes.begin(), s_Nodes.end(), [nodeId](auto& node) { return node.ID == nodeId; });
                        if (ed::AcceptDeletedItem())
                        {
                            // Отсоединяем ноду от всех подключенных к ней пинов
                            for (auto& input : id->Inputs) {
                                auto linkId = std::find_if(s_Links.begin(), s_Links.end(), [input](auto& link) { return link->EndPinID == input.ID; });
                                if (linkId != s_Links.end()) {
                                    s_Links.erase(linkId);
                                }
                            }
                            for (auto& output : id->Outputs) {
                                auto linkId = std::find_if(s_Links.begin(), s_Links.end(), [output](auto& link) { return link->StartPinID == output.ID; });
                                if (linkId != s_Links.end()) {
                                    s_Links.erase(linkId);
                                }
                            }
                            if (id != s_Nodes.end())
                                s_Nodes.erase(id);
                        }
                    }
                }
                ed::EndDelete();
            }

            ImGui::SetCursorScreenPos(cursorTopLeft);
        }


        auto currentMousePosition = ImGui::GetMousePos();

# if 0
        ed::Suspend();
        if (ed::ShowNodeContextMenu(&contextNodeId))
            ImGui::OpenPopup("Node Context Menu");
        else if (ed::ShowPinContextMenu(&contextPinId))
            ImGui::OpenPopup("Pin Context Menu");
        else if (ed::ShowLinkContextMenu(&contextLinkId))
            ImGui::OpenPopup("Link Context Menu");
        else if (ed::ShowBackgroundContextMenu())
        {
            ImGui::OpenPopup("Create New Node");
            newNodeLinkPin = nullptr;
        }
        ed::Resume();

        ed::Suspend();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        if (ImGui::BeginPopup("Node Context Menu"))
        {
            auto node = FindNode(contextNodeId);

            ImGui::TextUnformatted("Node Context Menu");
            ImGui::Separator();
            if (node)
            {
                ImGui::Text("ID: %p", node->ID.AsPointer());
                ImGui::Text("Type: %s", node->Type == NodeType::Blueprint ? "Blueprint" : (node->Type == NodeType::Tree ? "Tree" : "Comment"));
                ImGui::Text("Inputs: %d", (int)node->Inputs.size());
                ImGui::Text("Outputs: %d", (int)node->Outputs.size());
            }
            else
                ImGui::Text("Unknown node: %p", contextNodeId.AsPointer());
            ImGui::Separator();
            if (ImGui::MenuItem("Delete"))
                ed::DeleteNode(contextNodeId);
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopup("Pin Context Menu"))
        {
            auto pin = FindPin(contextPinId);

            ImGui::TextUnformatted("Pin Context Menu");
            ImGui::Separator();
            if (pin)
            {
                ImGui::Text("ID: %p", pin->ID.AsPointer());
                if (pin->Node)
                    ImGui::Text("Node: %p", pin->Node->ID.AsPointer());
                else
                    ImGui::Text("Node: %s", "<none>");
            }
            else
                ImGui::Text("Unknown pin: %p", contextPinId.AsPointer());

            ImGui::EndPopup();
        }

        if (ImGui::BeginPopup("Link Context Menu"))
        {
            auto link = FindLink(contextLinkId);

            ImGui::TextUnformatted("Link Context Menu");
            ImGui::Separator();
            if (link)
            {
                ImGui::Text("ID: %p", link->ID.AsPointer());
                ImGui::Text("From: %p", link->StartPinID.AsPointer());
                ImGui::Text("To: %p", link->EndPinID.AsPointer());
            }
            else
                ImGui::Text("Unknown link: %p", contextLinkId.AsPointer());
            ImGui::Separator();
            if (ImGui::MenuItem("Delete"))
                ed::DeleteLink(contextLinkId);
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopup("Create New Node"))
        {
            auto newNodePostion = currentMousePosition;
            //ImGui::SetCursorScreenPos(ImGui::GetMousePosOnOpeningCurrentPopup());

            //auto drawList = ImGui::GetWindowDrawList();
            //drawList->AddCircleFilled(ImGui::GetMousePosOnOpeningCurrentPopup(), 10.0f, 0xFFFF00FF);

            Node* node = nullptr;
            if (ImGui::MenuItem("Input Action"))
                node = SpawnInputActionNode();
            if (ImGui::MenuItem("Output Action"))
                node = SpawnOutputActionNode();
            if (ImGui::MenuItem("Branch"))
                node = SpawnBranchNode();
            if (ImGui::MenuItem("Do N"))
                node = SpawnDoNNode();
            if (ImGui::MenuItem("Set Timer"))
                node = SpawnSetTimerNode();
            if (ImGui::MenuItem("Less"))
                node = SpawnLessNode();
            if (ImGui::MenuItem("Weird"))
                node = SpawnWeirdNode();
            if (ImGui::MenuItem("Trace by Channel"))
                node = SpawnTraceByChannelNode();
            if (ImGui::MenuItem("Print String"))
                node = SpawnPrintStringNode();
            ImGui::Separator();
            if (ImGui::MenuItem("Comment"))
                node = SpawnComment();
            ImGui::Separator();
            if (ImGui::MenuItem("Sequence"))
                node = SpawnTreeSequenceNode();
            if (ImGui::MenuItem("Move To"))
                node = SpawnTreeTaskNode();
            if (ImGui::MenuItem("Random Wait"))
                node = SpawnTreeTask2Node();
            ImGui::Separator();
            if (ImGui::MenuItem("Message"))
                node = SpawnMessageNode();
            ImGui::Separator();
            if (ImGui::MenuItem("Transform"))
                node = SpawnHoudiniTransformNode();
            if (ImGui::MenuItem("Group"))
                node = SpawnHoudiniGroupNode();

            if (node)
            {
                BuildNodes();

                createNewNode = false;

                ed::SetNodePosition(node->ID, newNodePostion);

                if (auto startPin = newNodeLinkPin)
                {
                    auto& pins = startPin->Kind == PinKind::Input ? node->Outputs : node->Inputs;

                    for (auto& pin : pins)
                    {
                        if (CanCreateLink(startPin, &pin))
                        {
                            auto endPin = &pin;
                            if (startPin->Kind == PinKind::Input)
                                std::swap(startPin, endPin);

                            s_Links.emplace_back(Link(GetNextId(), startPin->ID, endPin->ID));
                            s_Links.back().Color = GetIconColor(startPin->Type);

                            break;
                        }
                    }
                }
            }

            ImGui::EndPopup();
        }
        else
            createNewNode = false;


        ImGui::PopStyleVar();
        ed::Resume();
# endif

        /*
            cubic_bezier_t c;
            c.p0 = pointf(100, 600);
            c.p1 = pointf(300, 1200);
            c.p2 = pointf(500, 100);
            c.p3 = pointf(900, 600);

            auto drawList = ImGui::GetWindowDrawList();
            auto offset_radius = 15.0f;
            auto acceptPoint = [drawList, offset_radius](const bezier_subdivide_result_t& r)
            {
                drawList->AddCircle(to_imvec(r.point), 4.0f, IM_COL32(255, 0, 255, 255));

                auto nt = r.tangent.normalized();
                nt = pointf(-nt.y, nt.x);

                drawList->AddLine(to_imvec(r.point), to_imvec(r.point + nt * offset_radius), IM_COL32(255, 0, 0, 255), 1.0f);
            };

            drawList->AddBezierCurve(to_imvec(c.p0), to_imvec(c.p1), to_imvec(c.p2), to_imvec(c.p3), IM_COL32(255, 255, 255, 255), 1.0f);
            cubic_bezier_subdivide(acceptPoint, c);
        */

        ed::End();

        if (restoreNodes) {
            restoreNodes = false;
        }

        if (ImGui::BeginDragDropTarget())
        {

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TREE_DRAG_CELL"))
            {
                auto transaction = document->BeginUndoTransaction("CreateNodeDialog"); //Коммит создания ноды

                Node* node = nullptr;

                IM_ASSERT(payload->DataSize == sizeof(uint32_t));
                uint32_t payload_D = *(const uint32_t*)payload->Data;
                node = CreateNode(payload_D);

                if (node)
                {
                    BuildNodes();

                    createNewNode = false;

                    ed::SetNodePosition(node->ID, currentMousePosition);

                    transaction->AddAction("%" PRI_node " created", FMT_node(node));
                }
            }
            else
                createNewNode = false;

            ImGui::EndDragDropTarget();
        }
    }
}

void rt3gui::Editor::ShowStyleEditor(bool* show)
{
    if (!ImGui::Begin("Style", show))
    {
        ImGui::End();
        return;
    }

    auto paneWidth = ImGui::GetContentRegionAvail();

    auto& editorStyle = ed::GetStyle();
    ImGui::BeginChild("Style buttons", paneWidth, true);
    ImGui::TextUnformatted("Values");
    //ImGui::Spring();
    if (ImGui::Button("Reset to defaults"))
        editorStyle = ed::Style();
    ImGui::EndChild();
    ImGui::Spacing();
    ImGui::DragFloat4("Node Padding", &editorStyle.NodePadding.x, 0.1f, 0.0f, 40.0f);
    ImGui::DragFloat("Node Rounding", &editorStyle.NodeRounding, 0.1f, 0.0f, 40.0f);
    ImGui::DragFloat("Node Border Width", &editorStyle.NodeBorderWidth, 0.1f, 0.0f, 15.0f);
    ImGui::DragFloat("Hovered Node Border Width", &editorStyle.HoveredNodeBorderWidth, 0.1f, 0.0f, 15.0f);
    ImGui::DragFloat("Selected Node Border Width", &editorStyle.SelectedNodeBorderWidth, 0.1f, 0.0f, 15.0f);
    ImGui::DragFloat("Pin Rounding", &editorStyle.PinRounding, 0.1f, 0.0f, 40.0f);
    ImGui::DragFloat("Pin Border Width", &editorStyle.PinBorderWidth, 0.1f, 0.0f, 15.0f);
    ImGui::DragFloat("Link Strength", &editorStyle.LinkStrength, 1.0f, 0.0f, 500.0f);
    //ImVec2  SourceDirection;
    //ImVec2  TargetDirection;
    ImGui::DragFloat("Scroll Duration", &editorStyle.ScrollDuration, 0.001f, 0.0f, 2.0f);
    ImGui::DragFloat("Flow Marker Distance", &editorStyle.FlowMarkerDistance, 1.0f, 1.0f, 200.0f);
    ImGui::DragFloat("Flow Speed", &editorStyle.FlowSpeed, 1.0f, 1.0f, 2000.0f);
    ImGui::DragFloat("Flow Duration", &editorStyle.FlowDuration, 0.001f, 0.0f, 5.0f);
    //ImVec2  PivotAlignment;
    //ImVec2  PivotSize;
    //ImVec2  PivotScale;
    //float   PinCorners;
    //float   PinRadius;
    //float   PinArrowSize;
    //float   PinArrowWidth;
    ImGui::DragFloat("Group Rounding", &editorStyle.GroupRounding, 0.1f, 0.0f, 40.0f);
    ImGui::DragFloat("Group Border Width", &editorStyle.GroupBorderWidth, 0.1f, 0.0f, 15.0f);

    ImGui::Separator();

    static ImGuiColorEditFlags edit_mode = ImGuiColorEditFlags_InputRGB;
    ImGui::BeginTable("Color Mode", 3);
    ImGui::TableNextColumn(); ImGui::TextUnformatted("Filter Colors");
    //ImGui::Spring();
    ImGui::TableNextColumn(); ImGui::RadioButton("RGB", &edit_mode, ImGuiColorEditFlags_InputRGB);
    //ImGui::Spring(0);
    ImGui::TableNextColumn(); ImGui::RadioButton("HSV", &edit_mode, ImGuiColorEditFlags_InputHSV);
    ImGui::EndTable();

    static ImGuiTextFilter filter;
    filter.Draw("##", paneWidth.x);

    ImGui::Spacing();

    ImGui::PushItemWidth(-160);
    for (int i = 0; i < ed::StyleColor_Count; ++i)
    {
        auto name = ed::GetStyleColorName((ed::StyleColor)i);
        if (!filter.PassFilter(name))
            continue;

        ImGui::ColorEdit4(name, &editorStyle.Colors[i].x, edit_mode);
    }
    ImGui::PopItemWidth();

    ImGui::End();
}

rt3gui::Node* rt3gui::Editor::CreateNode(uint32_t deviceId) {
    switch (deviceId) {
    case 1:
        return SpawnInputActionNode();
        break;
    case 2:
        return SpawnBranchNode();
        break;
    case 3:
        return SpawnDoNNode();
        break;
    case 4:
        return SpawnOutputActionNode();
        break;
    case 5:
        return SpawnSetTimerNode();
        break;
    case 6:
        return SpawnTreeSequenceNode();
        break;
    case 7:
        return SpawnTreeTaskNode();
        break;
    case 8:
        return SpawnTreeTask2Node();
        break;
    case 9:
        return SpawnLessNode();
        break;
    case 10:
        return SpawnWeirdNode();
        break;
    case 11:
        return SpawnMessageNode();
        break;
    case 12:
        return SpawnPrintStringNode();
        break;
    case 13:
        return SpawnHoudiniTransformNode();
        break;
    case 14:
        return SpawnHoudiniGroupNode();
        break;
    case 15:
        return SpawnTraceByChannelNode();
        break;
    case 16:
        return SpawnComment();
    }
}

rt3gui::Node* rt3gui::Editor::SpawnInputActionNode()
{
    s_Nodes.emplace_back(m_Generator.GenerateId(), "InputAction Fire", ImColor(255, 128, 128));
    s_Nodes.back().Outputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().Outputs.emplace_back(m_Generator.GenerateId(), "Pressed", SampleTypes::ComplexFloat);
    s_Nodes.back().Outputs.emplace_back(m_Generator.GenerateId(), "Released", SampleTypes::ComplexFloat);
    s_Nodes.back().devId = 1;
    BuildNode(&s_Nodes.back());

    return &s_Nodes.back();
}

rt3gui::Node* rt3gui::Editor::SpawnBranchNode()
{
    s_Nodes.emplace_back(m_Generator.GenerateId(), "Branch");
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "Condition", SampleTypes::FloatPoint);
    s_Nodes.back().Outputs.emplace_back(m_Generator.GenerateId(), "True", SampleTypes::FloatPoint);
    s_Nodes.back().Outputs.emplace_back(m_Generator.GenerateId(), "False", SampleTypes::FloatPoint);
    s_Nodes.back().devId = 2;
    BuildNode(&s_Nodes.back());

    return &s_Nodes.back();
}

rt3gui::Node* rt3gui::Editor::SpawnDoNNode()
{
    s_Nodes.emplace_back(m_Generator.GenerateId(), "Do N");
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "Enter", SampleTypes::FloatPoint);
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "N", SampleTypes::Integer);
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "Reset", SampleTypes::FloatPoint);
    s_Nodes.back().Outputs.emplace_back(m_Generator.GenerateId(), "Exit", SampleTypes::FloatPoint);
    s_Nodes.back().Outputs.emplace_back(m_Generator.GenerateId(), "Counter", SampleTypes::Integer);
    s_Nodes.back().devId = 3;
    BuildNode(&s_Nodes.back());

    return &s_Nodes.back();
}

rt3gui::Node* rt3gui::Editor::SpawnOutputActionNode()
{
    s_Nodes.emplace_back(m_Generator.GenerateId(), "OutputAction");
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "Sample", SampleTypes::FloatPoint);
    s_Nodes.back().Outputs.emplace_back(m_Generator.GenerateId(), "Condition", SampleTypes::FloatPoint);
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "Event", SampleTypes::FloatPoint);
    s_Nodes.back().devId = 4;
    BuildNode(&s_Nodes.back());

    return &s_Nodes.back();
}


rt3gui::Node* rt3gui::Editor::SpawnSetTimerNode()
{
    s_Nodes.emplace_back(m_Generator.GenerateId(), "Set Timer", ImColor(128, 195, 248));
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "Object", SampleTypes::ComplexFloat);
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "Function Name", SampleTypes::ComplexFloat);
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "Time", SampleTypes::FloatPoint);
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "Looping", SampleTypes::Integer);
    s_Nodes.back().Outputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().devId = 5;
    BuildNode(&s_Nodes.back());

    return &s_Nodes.back();
}

rt3gui::Node* rt3gui::Editor::SpawnTreeSequenceNode()
{
    s_Nodes.emplace_back(m_Generator.GenerateId(), "Sequence");
    s_Nodes.back().Type = NodeType::Tree;
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().Outputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().devId = 6;
    BuildNode(&s_Nodes.back());

    return &s_Nodes.back();
}

rt3gui::Node* rt3gui::Editor::SpawnTreeTaskNode()
{
    s_Nodes.emplace_back(m_Generator.GenerateId(), "Move To");
    s_Nodes.back().Type = NodeType::Tree;
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().devId = 7;
    BuildNode(&s_Nodes.back());

    return &s_Nodes.back();
}

rt3gui::Node* rt3gui::Editor::SpawnTreeTask2Node()
{
    s_Nodes.emplace_back(m_Generator.GenerateId(), "Random Wait");
    s_Nodes.back().Type = NodeType::Tree;
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().devId = 8;
    BuildNode(&s_Nodes.back());

    return &s_Nodes.back();
}

rt3gui::Node* rt3gui::Editor::SpawnLessNode()
{
    s_Nodes.emplace_back(m_Generator.GenerateId(), "<", ImColor(128, 195, 248));
    s_Nodes.back().Type = NodeType::Simple;
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().Outputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().devId = 9;
    BuildNode(&s_Nodes.back());

    return &s_Nodes.back();
}

rt3gui::Node* rt3gui::Editor::SpawnWeirdNode()
{
    s_Nodes.emplace_back(m_Generator.GenerateId(), "o.O", ImColor(128, 195, 248));
    s_Nodes.back().Type = NodeType::Simple;
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().Outputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().Outputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().devId = 10;
    BuildNode(&s_Nodes.back());

    return &s_Nodes.back();
}

rt3gui::Node* rt3gui::Editor::SpawnMessageNode()
{
    s_Nodes.emplace_back(m_Generator.GenerateId(), "", ImColor(128, 195, 248));
    s_Nodes.back().Type = NodeType::Simple;
    s_Nodes.back().Outputs.emplace_back(m_Generator.GenerateId(), "Message", SampleTypes::ComplexFloat);
    s_Nodes.back().devId = 11;
    BuildNode(&s_Nodes.back());

    return &s_Nodes.back();
}


rt3gui::Node* rt3gui::Editor::SpawnPrintStringNode()
{
    s_Nodes.emplace_back(m_Generator.GenerateId(), "Print String");
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "In String", SampleTypes::ComplexFloat);
    s_Nodes.back().Outputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().devId = 12;
    BuildNode(&s_Nodes.back());

    return &s_Nodes.back();
}

rt3gui::Node* rt3gui::Editor::SpawnHoudiniTransformNode()
{
    s_Nodes.emplace_back(m_Generator.GenerateId(), "Transform");
    s_Nodes.back().Type = NodeType::Houdini;
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().Outputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().devId = 13;
    BuildNode(&s_Nodes.back());

    return &s_Nodes.back();
}

rt3gui::Node* rt3gui::Editor::SpawnHoudiniGroupNode()
{
    s_Nodes.emplace_back(m_Generator.GenerateId(), "Group");
    s_Nodes.back().Type = NodeType::Houdini;
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().Outputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().devId = 14;
    BuildNode(&s_Nodes.back());

    return &s_Nodes.back();
}

rt3gui::Node* rt3gui::Editor::SpawnTraceByChannelNode()
{
    s_Nodes.emplace_back(m_Generator.GenerateId(), "Single Line Trace by Channel", ImColor(255, 128, 64));
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "Start", SampleTypes::FloatPoint);
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "End", SampleTypes::Integer);
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "Trace Channel", SampleTypes::FloatPoint);
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "Trace Complex", SampleTypes::Integer);
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "Actors to Ignore", SampleTypes::Integer);
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "Draw Debug Type", SampleTypes::Integer);
    s_Nodes.back().Inputs.emplace_back(m_Generator.GenerateId(), "Ignore Self", SampleTypes::Integer);
    s_Nodes.back().Outputs.emplace_back(m_Generator.GenerateId(), "", SampleTypes::FloatPoint);
    s_Nodes.back().Outputs.emplace_back(m_Generator.GenerateId(), "Out Hit", SampleTypes::FloatPoint);
    s_Nodes.back().Outputs.emplace_back(m_Generator.GenerateId(), "Return Value", SampleTypes::Integer);
    s_Nodes.back().devId = 15;
    BuildNode(&s_Nodes.back());

    return &s_Nodes.back();
}

rt3gui::Node* rt3gui::Editor::SpawnComment()
{
    s_Nodes.emplace_back(m_Generator.GenerateId(), "Test Comment");
    s_Nodes.back().Type = NodeType::Comment;
    s_Nodes.back().Size = ImVec2(300, 200);
    s_Nodes.back().devId = 16;
    return &s_Nodes.back();
}


bool rt3gui::Editor::Load(const crude_json::value& value, ed::Config& config)
{
    if (!value.is_object())
        return false;

    const crude_json::array* nodeArray = nullptr;
    if (!detail::GetPtrTo(value, "nodes", nodeArray)) // required
        return false;

    Editor editor{"TemporalEditor",textureLoader, config, document };

    IdGenerator generator;
    std::map<uint32_t, Pin*> pinMap;

    for (auto& nodeValue : *nodeArray)
    {
        uint32_t devId; 
        if (!detail::GetTo<crude_json::number>(nodeValue, "dev_id", devId)) // required
            return false;
        Node* node = editor.CreateNode(devId);

        if (!node->Load(nodeValue))
            return false;



        //// Collect pins for m_Link resolver
        //for (auto pin : node->GetInputPins())
        //    pinMap[pin->m_Id] = pin;
        //for (auto pin : node->GetOutputPins())
        //    pinMap[pin->m_Id] = pin;
    }

    const crude_json::object* stateObject = nullptr;
    if (!detail::GetPtrTo(value, "state", stateObject)) // required
        return false;

    uint32_t generatorState = 0;
    if (!detail::GetTo<crude_json::number>(*stateObject, "generator_state", generatorState)) // required
        return false;

    // load links
    const crude_json::array* linksArray = nullptr;
    if (!detail::GetPtrTo(value, "links", linksArray)) // required
        return false;
    for (auto& linkValue : *linksArray)
    {
        editor.s_Links.push_back(std::unique_ptr<Link>(new Link(linkValue)));
        auto startPin = editor.FindPin(editor.s_Links.back()->StartPinID);
        auto endPin = editor.FindPin(editor.s_Links.back()->EndPinID);
    }
    



    m_Generator.SetState(generatorState);

    s_Nodes.swap(editor.s_Nodes);
    s_Links.swap(editor.s_Links);
    restoreNodes = true;

    return true;
}

void rt3gui::Editor::Save(crude_json::value& value) const
{
    auto& nodesValue = value["nodes"]; // required
    nodesValue = crude_json::array();
    for (auto& node : s_Nodes)
    {
        crude_json::value nodeValue;

        nodeValue["type_id"] = crude_json::number(node.Type); // required
        nodeValue["dev_id"] = crude_json::number(node.devId); // required
        nodeValue["type_name"] = ToString(node.Type); // optional, to make data readable for humans

        node.Save(nodeValue);

        nodesValue.push_back(nodeValue);
    }

    auto& linksValue = value["links"]; // required
    linksValue = crude_json::array();
    for (auto& link : s_Links)
    {
        crude_json::value linkValue;

        linkValue["id"] = crude_json::number(link->ID.Get()); // required
        linkValue["start_id"] = crude_json::number(link->StartPinID.Get()); // required
        linkValue["end_id"] = crude_json::number(link->EndPinID.Get()); // required
        linkValue["color"] = crude_json::number(link->Color); // required
        linksValue.push_back(linkValue);
    }
    auto& stateValue = value["state"]; // required
    stateValue["generator_state"] = crude_json::number(m_Generator.State()); // required
}

bool rt3gui::Editor::Load(std::string_view path, ed::Config& config)
{
    auto value = crude_json::value::load(path.data());
    if (!value.second)
        return false;

    return Load(value.first, config);
}

bool rt3gui::Editor::Save(std::string_view path) const
{
    crude_json::value value;
    Save(value);

    return value.save(path.data(), 4);
}

bool rt3gui::Editor::startScheme() {
    return true;
}

bool rt3gui::Editor::pauseScheme() {
    return true;
}

bool rt3gui::Editor::stepScheme() {
    return true;
}

bool rt3gui::Editor::stopScheme() {
    return true;
}

bool rt3gui::Editor::isVisible() {
    return visible;
}

void rt3gui::Editor::makeVisible() {
    visible = true;
}

void rt3gui::Editor::makeInvisible() {
    visible = false;
}