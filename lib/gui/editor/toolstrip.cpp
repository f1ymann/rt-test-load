#include <toolstrip.h>

#include <string>

rt3gui::Action::Action(std::string_view name, OnTriggeredEvent::Delegate delegate)
    : m_Name(name)
{
    if (delegate)
        OnTriggered += std::move(delegate);
}

void rt3gui::Action::SetName(std::string_view name)
{
    if (m_Name == name)
        return;

    m_Name = name;

    OnChange(this);
}

const std::string& rt3gui::Action::GetName() const
{
    return m_Name;
}

void rt3gui::Action::SetEnabled(bool set)
{
    if (m_IsEnabled == set)
        return;

    m_IsEnabled = set;

    OnChange(this);
}

bool rt3gui::Action::IsEnabled() const
{
    return m_IsEnabled;
}

void rt3gui::Action::Execute()
{
    LOGV("Action: %s", m_Name.c_str());
    OnTriggered();
}

rt3gui::Toolstrip::Toolstrip(std::shared_ptr<rt3gui::TexturesLoader> _texturesLoader, std::map<std::string, Action>& _toolstripActions): toolstripActions(_toolstripActions)
{
    maxSamplesNum = 0;
    currentSamplesNum = 0;

    startState = false;
    pauseState = false;

    texturesLoader = _texturesLoader;
    executionTime = std::chrono::duration<double>(0);
#ifdef _WIN32
    std::string texturePrefix = "Data\\";
#else
    std::string texturePrefix = "Data/";
#endif // _WIN32


    for (auto& action : toolstripActions) {
        buttonTextures.emplace(action.first, texturesLoader->Application_LoadTexture((texturePrefix + action.first+".png").c_str()));
    }
}

rt3gui::Toolstrip::~Toolstrip()
{
}

void rt3gui::Toolstrip::drawToolstrip() {
    ImVec2 tex_size = ImVec2(32.0f, 32.0f);
    ImGui::BeginHorizontal("##ToolstripGroup", ImVec2(ImGui::GetWindowSize().x, 0));
    toolbarAction(toolstripActions["New"], tex_size);
    toolbarAction(toolstripActions["Undo"], tex_size);
    toolbarAction(toolstripActions["Redo"], tex_size);
    toolbarAction(toolstripActions["Open"], tex_size);
    toolbarAction(toolstripActions["Save"], tex_size);
    if (startState) {
        if (pauseState) {
            if(toolbarAction(toolstripActions["Unpause"], tex_size)){
                unpauseTime = std::chrono::system_clock::now();
            }
        }
        else {
            if(toolbarAction(toolstripActions["Pause"], tex_size)) {
                executionTime = executionTime + std::chrono::system_clock::now() - unpauseTime;
            }
        }
    }
    else {
        if(toolbarAction(toolstripActions["Start"], tex_size)) { 
            executionTime = std::chrono::duration<double>(0);
            unpauseTime = std::chrono::system_clock::now();
        }
    }
    toolbarAction(toolstripActions["Step"], tex_size);
    toolbarAction(toolstripActions["Stop"], tex_size);
    toolbarAction(toolstripActions["Settings"], tex_size);


    ImGui::Spring(1);

    std::string string;
    if (maxSamplesNum) {
        string = "Processed: " + std::to_string(currentSamplesNum) + " / " + std::to_string(maxSamplesNum) + " samples ";
    }
    else {
        string = "Processed: " + std::to_string(currentSamplesNum) + " samples ";
    }
    if (startState) {
        if (pauseState) {
            string += ", time spend " + std::to_string((executionTime).count()) + " s ";
        }
        else {
            string += ", time spend " + std::to_string(std::chrono::duration_cast<std::chrono::duration<double>>(executionTime + std::chrono::system_clock::now() - unpauseTime).count()) + " s ";
        }
    }
    else {
        if (executionTime != std::chrono::duration<double>(0)) {
            string += ", time spend " + std::to_string((executionTime).count()) + " s ";
        }
    }
    ImGui::TextUnformatted(string.c_str(), &string[string.size() - 1]);

    ImGui::Dummy(ImVec2(20, 0));
    ImGui::EndHorizontal();

}

void rt3gui::Toolstrip::setStartState(bool state) {
    if (startState && (!state)) { //Если система останавливается, запоминаем время ее работы
        executionTime = std::chrono::duration_cast<std::chrono::duration<double>>(executionTime + std::chrono::system_clock::now() - unpauseTime);
    }
    startState = state;
}

void rt3gui::Toolstrip::setPauseState(bool state) {
    pauseState = state;
}

bool rt3gui::Toolstrip::getStartState() {
    return startState;
}

bool rt3gui::Toolstrip::getPauseState() {
    return pauseState;
}

void rt3gui::Toolstrip::setCurrentSamplesNum(uint64_t samplesNum) {
    if (startState) {
        currentSamplesNum = samplesNum;
    }

}

void rt3gui::Toolstrip::setMaxSamplesNum(uint64_t samplesNum) {
    maxSamplesNum = samplesNum;
}


void rt3gui::Toolstrip::drawButton(const char* name, bool* flag, ImTextureID texture, ImVec2 texSize) {
    ImGui::PushID(name);
    int frame_padding = 0;                             // -1 == uses default padding (style.FramePadding)
    ImVec2 size = ImVec2(32.0f, 32.0f);// texSize;                     // Size of the image we want to make visible
    ImVec2 uv0 = ImVec2(0.0f, 0.0f);                        // UV coordinates for lower-left
    ImVec2 uv1 = ImVec2(32.0f / texSize.x, 32.0f / texSize.y);// UV coordinates for (32,32) in our texture
    ImVec4 bg_col = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];         // Button background
    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);       // No tint
    if (ImGui::ImageButton(texture, size, uv0, uv1, frame_padding, bg_col, tint_col))
        *flag = true;
    ImGui::PopID();
}

bool rt3gui::Toolstrip::toolbarAction(Action& action, ImVec2 texSize) {

    ImGui::PushID(action.GetName().c_str());
    int frame_padding = 0;                             // -1 == uses default padding (style.FramePadding)
    ImVec2 size = ImVec2(32.0f, 32.0f);// texSize;                     // Size of the image we want to make visible
    ImVec2 uv0 = ImVec2(0.0f, 0.0f);                        // UV coordinates for lower-left
    ImVec2 uv1 = ImVec2(32.0f / texSize.x, 32.0f / texSize.y);// UV coordinates for (32,32) in our texture
    ImVec4 bg_col = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];         // Button background
    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);       // No tint
    bool execute = false;

    ImGui::BeginDisabled(!action.IsEnabled());
    if (ImGui::ImageButton(buttonTextures[action.GetName()], size, uv0, uv1, frame_padding, bg_col, tint_col)) {
        action.Execute();
        execute = true;
    }
    ImGui::EndDisabled();
    ImGui::PopID();
    return execute;
}