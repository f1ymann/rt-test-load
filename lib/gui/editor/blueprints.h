#ifndef BLUEPRINTS_H
#define BLUEPRINTS_H
#include <App.h>
#include <editor.h>
#include <tree.h>
#include <toolstrip.h>
#include <schemeEditor.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <utility>

static ImEx::MostRecentlyUsedList Application_GetMostRecentlyOpenFileList()
{
    return ImEx::MostRecentlyUsedList("MostRecentlyOpenList");
}

struct Rt3Editor : App {
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    const char* Application_GetName();
    void Application_Initialize();
    void Application_Finalize();
    void Application_Frame();

    using App::App;

    void Start() override {
        Application_Initialize();
    }

    void Update() override {
        Application_Frame();
    }
private:
    void ShowDocumentsApp(bool* p_open);
    void ShowSettingsWindow();
    void NotifyOfDocumentsClosedElsewhere();
    void UpdateActions();
    void UpdateTitle();

    std::shared_ptr <rt3gui::TexturesLoader> texturesLoader;
    std::shared_ptr<rt3gui::Toolstrip> toolstrip;
    rt3gui::DevicesTree devicesTree;
    std::map<std::string, rt3gui::Document*> documents;
    std::string activeEditor;
    
    bool showSettingsWindow;
    uint32_t settingsBufferSize;
    uint64_t settingsSamplesToProcess;
    double settingsTd;
    double settingsFd;
    double settingsTdPrev;
    double settingsFdPrev;
    int unsavedDocsNum;

    std::map<std::string, rt3gui::Action> toolstripActions;
    crude_logger::OverlayLogger m_OverlayLogger;



    bool File_IsModified()
    {
        return documents[activeEditor]->m_IsModified;
    }

    void File_MarkModified()
    {
        if (documents[activeEditor]->m_IsModified)
            return;
        documents[activeEditor]->m_IsModified = true;
    }

    void File_New()
    {
        std::string name = "untitled" + std::to_string(++unsavedDocsNum) + ".rt";
        auto document = new rt3gui::Document(name.c_str(), texturesLoader, true);
        documents.insert(std::make_pair(name, document));
        activeEditor = name;
        documents[activeEditor]->OnMakeCurrent();
        UpdateTitle();
    }

    bool File_Open(std::string_view path, std::string* error = nullptr)
    {
        auto document = new rt3gui::Document(path.data(), texturesLoader, true);
        if (!document->Load(path))
        {
            if (error)
            {
                ImGuiTextBuffer buffer;
                buffer.appendf("Failed to load blueprint from file \"%" PRI_sv "\".", FMT_sv(path));
                *error = buffer.c_str();
            }
            else
                LOGE("Failed to load blueprint from file \"%" PRI_sv "\".", FMT_sv(path));
            return false;
        }


        crude_logger::OverlayLogger* loger = crude_logger::OverlayLogger::GetCurrent();
        LOGI("[File] Open \"%" PRI_sv "\"", FMT_sv(path));

        auto mostRecentlyOpenFiles = Application_GetMostRecentlyOpenFileList();
        mostRecentlyOpenFiles.Add(std::string(path));

        document->SetPath(path);
        document->DoOpen();

        documents.insert(std::make_pair(path, document));

        activeEditor = path;
        documents[activeEditor]->OnMakeCurrent();
        UpdateTitle();
        return true;
    }

    bool File_Open()
    {
        std::string path;
        std::vector<DialogFilter> filters{ { "RT3 scheme", "rt" } };
        OpenDialog(path, filters, documents.size() ? activeEditor.c_str() : "");
        
        if (path.empty())
            return false;

        std::string error;
        if (!File_Open(path, &error) && !error.empty())
        {
            //tinyfd_messageBox(
            //    (std::string(GetName()) + " - Open Blueprint...").c_str(),
            //    error.c_str(),
            //    "ok", "error", 1);

            LOGE("%s", error.c_str());

            return false;
        }

        return true;
    }

    bool File_Close(std::string docName)
    {
        std::string deletingFile = docName;
        if (activeEditor == deletingFile) { // Проверяем не было ли открытое окно первым и если нет, то делаем активным предыдущее
            auto it = std::find_if(documents.begin(), documents.end(), [deletingFile](const std::pair<std::string, rt3gui::Document*>& t) -> bool {return t.first != deletingFile; });
            if (it != documents.end()) {
                activeEditor = it->first;
            }
            else {
                activeEditor = "";
            }
        }
        documents.erase(deletingFile);
        return true;
    }

    bool File_Close() {
        return File_Close(activeEditor);
    }


    bool File_SaveAsEx(std::string_view path)
    {
        if(documents[activeEditor]->m_Path.empty()) {
            --unsavedDocsNum;
        }
        if (!documents.size())
            return true;

        if (!documents[activeEditor]->Save(path))
        {
            LOGE("Failed to save blueprint to file \"%" PRI_sv "\".", FMT_sv(path));
            return false;
        }

        documents[activeEditor]->m_IsModified = false;

        LOGI("[File] Save \"%" PRI_sv "\".", FMT_sv(path));

        return true;
    }

    bool File_SaveAs()
    {
        std::string path;
        std::vector<DialogFilter> filters{ { "RT3 scheme", "rt" } };
        SaveDialog(path, filters, documents[activeEditor]->m_Path);
        //const char* filterPatterns[1] = { "*.rt" };
        //auto path = tinyfd_saveFileDialog(
        //    "Save Blueprint...",
        //    m_Document->m_Path.c_str(),
        //    1, filterPatterns,
        //    "Blueprint Files (*.bp)");
        if (path.empty())
            return false;

        if (!File_SaveAsEx(path))
            return false;

        documents[activeEditor]->SetPath(path);

        auto docHandler = documents.extract(activeEditor);
        docHandler.key() = path;
        documents.insert(std::move(docHandler)); 
        activeEditor = path;

        UpdateTitle();

        return true;
    }

    bool File_Save(std::string docName)
    {
        if (!documents[docName]->m_Path.empty())
            return File_SaveAsEx(documents[docName]->m_Path);
        else {
            return File_SaveAs();
        }
        return true;
    }

    bool File_Save() {
        return File_Save(activeEditor); // Если сохраняем файл не указывая какой, то по умолчанию это будет активный файл
    }

    void File_Exit()
    {

    }

    void Edit_Undo()
    {
        documents[activeEditor]->Undo();
    }

    void Edit_Redo()
    {
        documents[activeEditor]->Redo();
    }

    void Edit_Cut()
    {
    }

    void Edit_Copy()
    {
    }

    void Edit_Paste()
    {
    }

    void Edit_Duplicate()
    {
    }

    void Edit_Delete()
    {
    }

    void Edit_SelectAll()
    {
    }

    void View_NavigateBackward()
    {
    }

    void View_NavigateForward()
    {
    }

    void Blueprint_Start()
    {
        documents[activeEditor]->Start();
    }

    void Blueprint_Step()
    {
        documents[activeEditor]->Step();
    }

    void Blueprint_Stop()
    {
        documents[activeEditor]->Stop();
    }

    void Blueprint_Pause()
    {
        documents[activeEditor]->Pause();
    }
    void Open_Settings() {
        showSettingsWindow = true;
    }
};

#endif //BLUEPRINTS_H