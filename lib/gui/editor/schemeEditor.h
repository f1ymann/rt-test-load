#ifndef SCHEME_EDITOR_H
#define SCHEME_EDITOR_H

#include <imgui.h>

#include <editor.h>
#include <toolstrip.h>
#include <crude_json.h>

namespace rt3gui {

    struct Document
    {
        Document(const char* name, std::shared_ptr < rt3gui::TexturesLoader> _textureLoader, bool open = true, const ImVec4& color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        void DoOpen() { Open = true; }
        void DoOpenPrev() { OpenPrev = true; }
        void DoQueueClose() { WantClose = true; }
        //void DoForceClose() { Open = false; Dirty = false; }
        //void DoSave() { Dirty = false; }
        void DisplayContents(std::shared_ptr<rt3gui::Toolstrip> toolstrip, std::string& _activeEditor);

        void makeEditorVisible();
        void makeEditorInvisible();

        const char* getName() { return m_Name.c_str(); };
        bool getOpenPrev() { return OpenPrev; };
        bool getDirty() { return m_IsModified; };
        bool getWantClose() { return WantClose; };
        ImVec4 getColor() { return Color; };


        void setOpenPrev(bool val) { OpenPrev = val; };
        void setWantClose(bool val) { WantClose = val; };
        void InstallDocumentCallbacks(ed::Config& config);


        struct DocumentState
        {
            crude_json::value m_NodesState;
            crude_json::value m_BlueprintState;

            crude_json::value Serialize() const;

            static bool Deserialize(const crude_json::value& value, DocumentState& result);
        };

        struct NavigationState
        {
            crude_json::value                   m_ViewState;
        };

        struct UndoState
        {
            std::string          m_Name;
            DocumentState   m_State;
        };

        struct UndoTransaction
            : std::enable_shared_from_this<UndoTransaction>
        {
            UndoTransaction(Document& document, std::string_view name);
            UndoTransaction(UndoTransaction&&) = delete;
            UndoTransaction(const UndoTransaction&) = delete;
            ~UndoTransaction();

            UndoTransaction& operator=(UndoTransaction&&) = delete;
            UndoTransaction& operator=(const UndoTransaction&) = delete;

            void Begin(std::string_view name = "");
            void AddAction(std::string_view name);
            void AddAction(const char* format, ...) IM_FMTARGS(2);
            void Commit(std::string_view name = "");
            void Discard();

            const Document* GetDocument() const { return m_Document; }

        private:
            std::string                      m_Name;
            Document* m_Document = nullptr;
            UndoState                   m_State;
            ImGuiTextBuffer             m_Actions;
            bool                        m_HasBegan = false;
            bool                        m_IsDone = false;
            std::shared_ptr<UndoTransaction> m_MasterTransaction;
        };

# if !CRUDE_BP_MSVC2015 // [[nodiscard]] is unrecognized attribute
        [[nodiscard]]
# endif
        std::shared_ptr<UndoTransaction> BeginUndoTransaction(std::string_view name, std::string_view action = "");

# if !CRUDE_BP_MSVC2015 // [[nodiscard]] is unrecognized attribute
        [[nodiscard]]
# endif
        std::shared_ptr<UndoTransaction> GetDeferredUndoTransaction(std::string_view name);

        void SetPath(std::string_view path);

        crude_json::value Serialize() const;

        bool Deserialize(const crude_json::value& value);

        bool Load(std::string_view path);
        bool Save(std::string_view path);

        bool Undo();
        bool Redo();

        DocumentState BuildDocumentState();
        void ApplyState(const DocumentState& state);
        void ApplyState(const NavigationState& state);

        void OnMakeCurrent();

        void OnSaveBegin();
        bool OnSaveNodeState(uint32_t nodeId, const char* data, size_t size, ed::SaveReasonFlags reason);
        bool OnSaveState(const char* data, size_t size, ed::SaveReasonFlags reason);
        void OnSaveEnd();

        size_t OnLoadNodeState(uint32_t nodeId, char* data) const;
        size_t OnLoadState(char* data) const;

        Editor& GetEditor() { return *m_Editor; }
        const Editor& GetEditor() const { return *m_Editor; }

        std::string                  m_Path;
        std::string                  m_Name;
        bool                    m_IsModified = false;
        std::vector<UndoState>       m_Undo;
        std::vector<UndoState>       m_Redo;

        DocumentState           m_DocumentState;
        NavigationState         m_NavigationState;

        UndoTransaction* m_MasterTransaction = nullptr;

        std::shared_ptr<UndoTransaction> m_SaveTransaction = nullptr;

        std::unique_ptr<Editor>               m_Editor;
        std::shared_ptr < rt3gui::TexturesLoader> m_TextureLoader;


        void Start() { m_Editor->startScheme(); }
        void Pause() { m_Editor->pauseScheme(); }
        void Stop() { m_Editor->stopScheme(); }
        void Step() { m_Editor->stepScheme(); }


    public:
        bool        Open;       // Set when open


    private:
        bool        OpenPrev;   // Copy of Open from last update.
        bool        WantClose;  // Set when the document
        ImVec4      Color;      // An arbitrary variable associated to the document
    };

} // rt3gui
#endif //SCHEME_EDITOR_H