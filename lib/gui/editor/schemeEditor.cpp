#include <schemeEditor.h>
# include <crude_logger.h>


rt3gui::Document::Document(const char* name, std::shared_ptr < rt3gui::TexturesLoader> _textureLoader, bool open, const ImVec4& color)
{
    m_TextureLoader = _textureLoader;
    m_Name = name;
    Open = open;
    Color = color;
    WantClose = false;
    ed::Config config;
    InstallDocumentCallbacks(config);

    m_Editor = std::unique_ptr<Editor>(new Editor(name, _textureLoader, config, this));
}


void rt3gui::Document::InstallDocumentCallbacks(ed::Config& config)
{
    config.UserPointer = this;
    config.BeginSaveSession = [](void* userPointer)
    {
        auto self = reinterpret_cast<Document*>(userPointer);

        if (self)
            self->OnSaveBegin();
    };
    config.EndSaveSession = [](void* userPointer)
    {
        auto self = reinterpret_cast<Document*>(userPointer);

        if (self)
            self->OnSaveEnd();
    };
    config.SaveSettings = [](const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer) -> bool
    {
        auto self = reinterpret_cast<Document*>(userPointer);

        if (self)
            return self->OnSaveState(data, size, reason);
        else
            return false;
    };
    config.LoadSettings = [](char* data, void* userPointer) -> size_t
    {
        auto self = reinterpret_cast<Document*>(userPointer);

        if (self)
            return self->OnLoadState(data);
        else
            return {};
    };
    config.SaveNodeSettings = [](ed::NodeId nodeId, const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer) -> bool
    {
        auto self = reinterpret_cast<Document*>(userPointer);

        if (self)
            return self->OnSaveNodeState(static_cast<uint32_t>(nodeId.Get()), data, size, reason);
        else
            return false;
    };
    config.LoadNodeSettings = [](ed::NodeId nodeId, char* data, void* userPointer) -> size_t
    {
        auto self = reinterpret_cast<Document*>(userPointer);

        if (self)
            return self->OnLoadNodeState(static_cast<uint32_t>(nodeId.Get()), data);
        else
            return {};
    };
}

// Display placeholder contents for the Document
void rt3gui::Document::DisplayContents(std::shared_ptr<rt3gui::Toolstrip> toolstrip, std::string& _activeEditor)
{
    ImGui::PushID(m_Name.c_str());

    m_Editor->drawEditor();
    ImGui::PopID();
}


void rt3gui::Document::makeEditorVisible() {
    m_Editor->makeVisible();
}


void rt3gui::Document::makeEditorInvisible() {
    m_Editor->makeInvisible();
}


namespace rt3gui {

    static std::string SaveReasonFlagsToString(ed::SaveReasonFlags flags, std::string_view separator = ", ")
    {
        ImGuiTextBuffer builder;

        auto testFlag = [flags](ed::SaveReasonFlags flag)
        {
            return (flags & flag) == flag;
        };

        if (testFlag(ed::SaveReasonFlags::Navigation))
            builder.appendf("Navigation%" PRI_sv, FMT_sv(separator));
        if (testFlag(ed::SaveReasonFlags::Position))
            builder.appendf("Position%" PRI_sv, FMT_sv(separator));
        if (testFlag(ed::SaveReasonFlags::Size))
            builder.appendf("Size%" PRI_sv, FMT_sv(separator));
        if (testFlag(ed::SaveReasonFlags::Selection))
            builder.appendf("Selection%" PRI_sv, FMT_sv(separator));
        if (testFlag(ed::SaveReasonFlags::User))
            builder.appendf("User%" PRI_sv, FMT_sv(separator));

        if (builder.empty())
            return "None";
        else
            return std::string(builder.c_str(), builder.size() - separator.size());
    }

} // namespace rt3gui



crude_json::value rt3gui::Document::DocumentState::Serialize() const
{
    crude_json::value result;
    result["nodes"] = m_NodesState;
    result["blueprint"] = m_BlueprintState;
    return result;
}

bool rt3gui::Document::DocumentState::Deserialize(const crude_json::value& value, DocumentState& result)
{
    DocumentState state;

    if (!value.is_object())
        return false;

    if (!value.contains("nodes") || !value.contains("blueprint"))
        return false;

    auto& nodesValue = value["nodes"];
    auto& dataValue = value["blueprint"];

    if (!nodesValue.is_array())
        return false;

    state.m_NodesState = nodesValue;
    state.m_BlueprintState = dataValue;

    result = std::move(state);

    return true;
}





rt3gui::Document::UndoTransaction::UndoTransaction(Document& document, std::string_view name)
    : m_Name(name)
    , m_Document(&document)
{
}

rt3gui::Document::UndoTransaction::~UndoTransaction()
{
    Commit();
}

void rt3gui::Document::UndoTransaction::Begin(std::string_view name /*= ""*/)
{
    if (m_HasBegan)
        return;

    if (m_Document->m_MasterTransaction)
    {
        m_MasterTransaction = m_Document->m_MasterTransaction->shared_from_this();
    }
    else
    {
        // Spawn master transaction which commit on destruction
        m_MasterTransaction = m_Document->GetDeferredUndoTransaction(m_Name);
        m_Document->m_MasterTransaction = m_MasterTransaction.get();
        m_MasterTransaction->Begin();
        m_MasterTransaction->m_MasterTransaction = nullptr;
    }

    m_HasBegan = true;

    m_State.m_State = m_Document->m_DocumentState;

    if (!name.empty())
        AddAction(name);
}

void rt3gui::Document::UndoTransaction::AddAction(std::string_view name)
{
    if (!m_HasBegan || name.empty())
        return;

    m_Actions.appendf("%" PRI_sv "\n", FMT_sv(name));
}

void rt3gui::Document::UndoTransaction::AddAction(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    ImGuiTextBuffer buffer;
    buffer.appendfv(format, args);
    va_end(args);

    AddAction(std::string_view(buffer.Buf.Data, buffer.Buf.Size));
}

void rt3gui::Document::UndoTransaction::Commit(std::string_view name)
{
    if (!m_HasBegan || m_IsDone)
        return;

    if (m_MasterTransaction)
    {
        if (!name.empty())
        {
            m_MasterTransaction->m_Actions.append(name.data(), name.data() + name.size());
            m_MasterTransaction->m_Actions.append("\n");
        }
        else
        {
            m_MasterTransaction->m_Actions.append(m_Actions.c_str());
        }

        IM_ASSERT(m_MasterTransaction->m_IsDone == false);
    }
    else
    {
        IM_ASSERT(m_Document->m_MasterTransaction == this);

        if (!m_Actions.empty())
        {
            if (name.empty())
                name = std::string_view(m_Actions.c_str(), m_Actions.size() - 1);

            LOGV("[UndoTransaction] Commit: %" PRI_sv, FMT_sv(name));

            m_State.m_Name = name;

            m_Document->m_Undo.emplace_back(std::move(m_State));
            m_Document->m_Redo.clear();

            m_Document->m_DocumentState = m_Document->BuildDocumentState();
        }

        m_Document->m_MasterTransaction = nullptr;
    }

    m_IsDone = true;
}

void rt3gui::Document::UndoTransaction::Discard()
{
    if (!m_HasBegan || m_IsDone)
        return;

    if (!m_MasterTransaction)
    {
        IM_ASSERT(m_Document->m_MasterTransaction == this);

        m_Document->m_MasterTransaction = nullptr;
    }

    m_IsDone = true;
}





void rt3gui::Document::OnSaveBegin()
{
    m_SaveTransaction = BeginUndoTransaction("Save");
}

bool rt3gui::Document::OnSaveNodeState(uint32_t nodeId, const char* data, size_t size, ed::SaveReasonFlags reason)
{
    m_DocumentState.m_NodesState[nodeId] = std::string(data, size);

    if (reason != ed::SaveReasonFlags::Size)
    {
        auto node = m_Editor->FindNode(nodeId);

        ImGuiTextBuffer buffer;
        buffer.appendf("%" PRI_node " %s", FMT_node(node), SaveReasonFlagsToString(reason).c_str());
        m_SaveTransaction->AddAction(buffer.c_str());
    }

    return true;
}

bool rt3gui::Document::OnSaveState(const char* data, size_t size, ed::SaveReasonFlags reason)
{
    m_NavigationState.m_ViewState = std::string(data, size);
    return true;
}


void rt3gui::Document::OnSaveEnd()
{
    m_SaveTransaction = nullptr;

    m_DocumentState = BuildDocumentState();
}

size_t rt3gui::Document::OnLoadNodeState(uint32_t nodeId, char* data) const
{
    if (m_DocumentState.m_NodesState.is_array()) {
        size_t size = m_DocumentState.m_NodesState[nodeId].get<std::string>().size();
        if (data) {
            m_DocumentState.m_NodesState[nodeId].get<std::string>().copy(data, size, 0);
            return size;
        }
        else {
            return size;
        }
    }
    return 0;
}

size_t rt3gui::Document::OnLoadState(char* data) const
{
    if (m_NavigationState.m_ViewState.is_string()) {
        size_t size = m_NavigationState.m_ViewState.get<std::string>().size();
        if (data) {
            m_NavigationState.m_ViewState.get<std::string>().copy(data, size, 0);
            return size;
        }
        else {
            return size;
        }
    }
    return 0;
}

std::shared_ptr<rt3gui::Document::UndoTransaction> rt3gui::Document::BeginUndoTransaction(std::string_view name, std::string_view action /*= ""*/)
{
    auto transaction = std::make_shared<UndoTransaction>(*this, name);
    transaction->Begin(action);
    m_IsModified = true;
    return transaction;
}

std::shared_ptr<rt3gui::Document::UndoTransaction> rt3gui::Document::GetDeferredUndoTransaction(std::string_view name)
{
    return std::make_shared<UndoTransaction>(*this, name);
}

void rt3gui::Document::SetPath(std::string_view path)
{
    m_Path = path;

    auto lastSeparator = m_Path.find_last_of("\\/");
    if (lastSeparator != std::string::npos)
        m_Name = m_Path.substr(lastSeparator + 1);
    else
        m_Name = path;
}

crude_json::value rt3gui::Document::Serialize() const
{
    crude_json::value result;
    result["document"] = m_DocumentState.Serialize();
    result["view"] = m_NavigationState.m_ViewState;
    return result;
}

bool rt3gui::Document::Deserialize(const crude_json::value& value)
{
    if (!value.is_object())
        return false;

    if (!value.contains("document") || !value.contains("view"))
        return false;

    auto& documentValue = value["document"];
    auto& viewValue = value["view"];

    //Document document(m_Name.c_str(), m_LibraryLoader, m_TextureLoader);
    if (!DocumentState::Deserialize(documentValue, m_DocumentState))
        return false;

    m_NavigationState.m_ViewState = viewValue;
    ed::Config config;
    InstallDocumentCallbacks(config);
    if (!m_Editor->Load(m_DocumentState.m_BlueprintState, config))
        return false;


    return true;
}

bool rt3gui::Document::Load(std::string_view path)
{
    auto loadResult = crude_json::value::load(path.data());
    if (!loadResult.second)
        return false;

    Document document(m_Name.c_str(), m_TextureLoader);
    *this = std::move(document);
    ed::Config config;
    InstallDocumentCallbacks(config);
    m_Editor = std::unique_ptr<Editor>(new Editor(m_Name.c_str(), m_TextureLoader, config, this));
    m_IsModified = false;

    if (!Deserialize(loadResult.first))
        return false;

    return true;
}

bool rt3gui::Document::Save(std::string_view path)
{
    m_IsModified = false;
    m_Editor->setCurrentEditor();
    auto result = Serialize();
    return result.save(path.data());
}

bool rt3gui::Document::Undo()
{
    if (m_Undo.empty())
        return false;

    auto state = std::move(m_Undo.back());
    m_Undo.pop_back();

    LOGI("[Document] Undo: %s", state.m_Name.c_str());

    UndoState undoState;
    undoState.m_Name = state.m_Name;
    undoState.m_State = m_DocumentState;

    ApplyState(state.m_State);

    m_Redo.push_back(std::move(undoState));
    m_IsModified = true;

    return true;
}

bool rt3gui::Document::Redo()
{
    if (m_Redo.empty())
        return true;

    auto state = std::move(m_Redo.back());
    m_Redo.pop_back();

    LOGI("[Document] Redo: %s", state.m_Name.c_str());

    UndoState undoState;
    undoState.m_Name = state.m_Name;
    undoState.m_State = m_DocumentState;

    ApplyState(state.m_State);

    m_Undo.push_back(std::move(undoState));
    m_IsModified = true;

    return true;
}

rt3gui::Document::DocumentState rt3gui::Document::BuildDocumentState()
{
    DocumentState result;
    m_Editor->Save(result.m_BlueprintState);
    result.m_NodesState = m_DocumentState.m_NodesState;
    return result;
}

void rt3gui::Document::ApplyState(const NavigationState& state)
{
    m_NavigationState = state;
}

void rt3gui::Document::ApplyState(const DocumentState& state)
{
    ed::Config config;
    InstallDocumentCallbacks(config);
    m_Editor->Load(state.m_BlueprintState, config);
    m_Editor->setCurrentEditor();
    ed::ClearSelection();
    m_DocumentState = state;
}

void rt3gui::Document::OnMakeCurrent()
{
    ApplyState(m_DocumentState);
    ApplyState(m_NavigationState);
}
