#include <blueprints.h>


const char* Rt3Editor::Application_GetName()
{
    return "RT3 Editor";
}

void Rt3Editor::Application_Finalize()
{
    
}


static bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f)
{
    using namespace ImGui;
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID("##Splitter");
    ImRect bb;
    bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
    bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
    return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
}


void Rt3Editor::Application_Initialize()
{
    showSettingsWindow = false;

#ifdef _WIN32
    texturesLoader = std::shared_ptr <rt3gui::TexturesLoader>(new rt3gui::TexturesLoader(glob_g_pd3dDevice));
#else
    texturesLoader = std::shared_ptr <rt3gui::TexturesLoader>(new rt3gui::TexturesLoader());
#endif // _WIN32

    devicesTree = rt3gui::DevicesTree();

    unsavedDocsNum = 0;
    //std::string name = "untitled" + std::to_string(unsavedDocsNum) + ".rt";
    //documents.emplace(std::make_pair(name,new rt3gui::Document(name.c_str(), libraryLoader, texturesLoader, true, ImVec4(0.4f, 0.8f, 0.4f, 1.0f))));
    //activeEditor = name;
    settingsBufferSize = 0;// documents[activeEditor]->getSchemeProperties()->bufferSize;
    settingsSamplesToProcess = 0;// documents[activeEditor]->getSchemeProperties()->systemTactsToProcess* settingsBufferSize;


    toolstripActions.emplace(std::make_pair("New", rt3gui::Action{ "New",          [this] { File_New();    } }));
    toolstripActions.emplace(std::make_pair("Open", rt3gui::Action{"Open",      [this] { File_Open();   }}));
    toolstripActions.emplace(std::make_pair("Save As", rt3gui::Action { "Save As",   [this] { File_SaveAs(); } }));
    toolstripActions.emplace(std::make_pair("Save", rt3gui::Action { "Save",         [this] { File_Save();   } }));
    toolstripActions.emplace(std::make_pair("Close", rt3gui::Action { "Close",        [this] { File_Close();  } }));
    toolstripActions.emplace(std::make_pair("Exit", rt3gui::Action { "Exit",         [this] { File_Exit();   } }));

    toolstripActions.emplace(std::make_pair("Undo", rt3gui::Action { "Undo",         [this] { Edit_Undo();      } }));
    toolstripActions.emplace(std::make_pair("Redo", rt3gui::Action { "Redo",         [this] { Edit_Redo();      } }));
    toolstripActions.emplace(std::make_pair("Cut", rt3gui::Action { "Cut",          [this] { Edit_Cut();       } }));
    toolstripActions.emplace(std::make_pair("Copy", rt3gui::Action { "Copy",         [this] { Edit_Copy();      } }));
    toolstripActions.emplace(std::make_pair("Paste", rt3gui::Action { "Paste",        [this] { Edit_Paste();     } }));
    toolstripActions.emplace(std::make_pair("Duplicate", rt3gui::Action { "Duplicate",    [this] { Edit_Duplicate(); } }));
    toolstripActions.emplace(std::make_pair("Delete", rt3gui::Action { "Delete",       [this] { Edit_Delete();    } }));
    toolstripActions.emplace(std::make_pair("Select All", rt3gui::Action { "Select All",   [this] { Edit_SelectAll(); } }));

    toolstripActions.emplace(std::make_pair("Navigate Backward", rt3gui::Action { "Navigate Backward", [this] { View_NavigateBackward(); } }));
    toolstripActions.emplace(std::make_pair("Navigate Forward", rt3gui::Action { "Navigate Forward",  [this] { View_NavigateForward();  } }));

    toolstripActions.emplace(std::make_pair("Start", rt3gui::Action { "Start",        [this] { Blueprint_Start(); } }));
    toolstripActions.emplace(std::make_pair("Pause", rt3gui::Action{ "Pause",         [this] { Blueprint_Pause();  } }));
    toolstripActions.emplace(std::make_pair("Unpause", rt3gui::Action{ "Unpause",         [this] { Blueprint_Pause();  } }));
    toolstripActions.emplace(std::make_pair("Step", rt3gui::Action { "Step",         [this] { Blueprint_Step();  } }));
    toolstripActions.emplace(std::make_pair("Stop", rt3gui::Action { "Stop",         [this] { Blueprint_Stop();  } }));

    toolstripActions.emplace(std::make_pair("Settings", rt3gui::Action{ "Settings",         [this] { Open_Settings();  } }));


    toolstrip = std::shared_ptr<rt3gui::Toolstrip>(new rt3gui::Toolstrip(texturesLoader, toolstripActions));

    crude_logger::OverlayLogger::SetCurrent(&m_OverlayLogger);

    m_OverlayLogger.AddKeyword("Node");
    m_OverlayLogger.AddKeyword("Pin");
    m_OverlayLogger.AddKeyword("Link");
    m_OverlayLogger.AddKeyword("CreateNodeDialog");
    m_OverlayLogger.AddKeyword("NodeContextMenu");
    m_OverlayLogger.AddKeyword("PinContextMenu");
    m_OverlayLogger.AddKeyword("LinkContextMenu");

    m_OverlayLogger.AddKeyword("SpawnInputActionNode");
    m_OverlayLogger.AddKeyword("SpawnBranchNode");
    m_OverlayLogger.AddKeyword("SpawnDoNNode");
    m_OverlayLogger.AddKeyword("SpawnOutputActionNode");
    m_OverlayLogger.AddKeyword("SpawnSetTimerNode");
    m_OverlayLogger.AddKeyword("SpawnTreeSequenceNode");
    m_OverlayLogger.AddKeyword("SpawnTreeTaskNode");
    m_OverlayLogger.AddKeyword("SpawnTreeTask2Node");
    m_OverlayLogger.AddKeyword("SpawnLessNode");
    m_OverlayLogger.AddKeyword("SpawnWeirdNode");
    m_OverlayLogger.AddKeyword("SpawnMessageNode");
    m_OverlayLogger.AddKeyword("SpawnPrintStringNode");
    m_OverlayLogger.AddKeyword("SpawnHoudiniTransformNode");
    m_OverlayLogger.AddKeyword("SpawnHoudiniGroupNode");
    m_OverlayLogger.AddKeyword("SpawnTraceByChannelNode");

    ImEx::MostRecentlyUsedList::Install(ImGui::GetCurrentContext());

}

void DrawMenuBar() {
    // Menu Bar
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            ImGui::MenuItem("(demo menu)", NULL, false, false);
            if (ImGui::MenuItem("New")) {}
            if (ImGui::MenuItem("Open", "Ctrl+O")) {}
            if (ImGui::BeginMenu("Open Recent"))
            {
                ImGui::MenuItem("fish_hat.c");
                ImGui::MenuItem("fish_hat.inl");
                ImGui::MenuItem("fish_hat.h");
                if (ImGui::BeginMenu("More.."))
                {
                    ImGui::MenuItem("Hello");
                    ImGui::MenuItem("Sailor");
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {}
            if (ImGui::MenuItem("Save As..")) {}

            ImGui::Separator();
            if (ImGui::BeginMenu("Options"))
            {
                static bool enabled = true;
                ImGui::MenuItem("Enabled", "", &enabled);
                ImGui::BeginChild("child", ImVec2(0, 60), true);
                for (int i = 0; i < 10; i++)
                    ImGui::Text("Scrolling Text %d", i);
                ImGui::EndChild();
                static float f = 0.5f;
                static int n = 0;
                ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
                ImGui::InputFloat("Input", &f, 0.1f);
                ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Colors"))
            {
                float sz = ImGui::GetTextLineHeight();
                for (int i = 0; i < ImGuiCol_COUNT; i++)
                {
                    const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
                    ImVec2 p = ImGui::GetCursorScreenPos();
                    ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
                    ImGui::Dummy(ImVec2(sz, sz));
                    ImGui::SameLine();
                    ImGui::MenuItem(name);
                }
                ImGui::EndMenu();
            }

            // Here we demonstrate appending again to the "Options" menu (which we already created above)
            // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
            // In a real code-base using it would make senses to use this feature from very different code locations.
            if (ImGui::BeginMenu("Options")) // <-- Append!
            {
                static bool b = true;
                ImGui::Checkbox("SomeOption", &b);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Disabled", false)) // Disabled
            {
                IM_ASSERT(0);
            }
            if (ImGui::MenuItem("Checked", NULL, true)) {}
            if (ImGui::MenuItem("Quit", "Alt+F4")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Examples"))
        {
            ImGui::EndMenu();
        }
        //if (ImGui::MenuItem("MenuItem")) {} // You can also use MenuItem() inside a menu bar!
        if (ImGui::BeginMenu("Tools"))
        {
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}




// [Optional] Notify the system of Tabs/Windows closure that happened outside the regular tab interface.
// If a tab has been closed programmatically (aka closed from another source such as the Checkbox() in the demo,
// as opposed to clicking on the regular tab closing button) and stops being submitted, it will take a frame for
// the tab bar to notice its absence. During this frame there will be a gap in the tab bar, and if the tab that has
// disappeared was the selected one, the tab bar will report no selected tab during the frame. This will effectively
// give the impression of a flicker for one frame.
// We call SetTabItemClosed() to manually notify the Tab Bar or Docking system of removed tabs to avoid this glitch.
// Note that this completely optional, and only affect tab bars with the ImGuiTabBarFlags_Reorderable flag.
void Rt3Editor::NotifyOfDocumentsClosedElsewhere()
{
    for (auto& doc : documents)
    {
        ;
        if (!doc.second->Open && doc.second->getOpenPrev())
            ImGui::SetTabItemClosed(doc.first.c_str());
        doc.second->setOpenPrev(doc.second->Open);
    }
}

void Rt3Editor::ShowDocumentsApp(bool* p_open)
{
    static bool opt_reorderable = true;
    static ImGuiTabBarFlags opt_fitting_flags = ImGuiTabBarFlags_FittingPolicyDefault_;


    // About the ImGuiWindowFlags_UnsavedDocument / ImGuiTabItemFlags_UnsavedDocument flags.
    // They have multiple effects:
    // - Display a dot next to the title.
    // - Tab is selected when clicking the X close button.
    // - Closure is not assumed (will wait for user to stop submitting the tab).
    //   Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
    //   We need to assume closure by default otherwise waiting for "lack of submission" on the next frame would leave an empty
    //   hole for one-frame, both in the tab-bar and in tab-contents when closing a tab/window.
    //   The rarely used SetTabItemClosed() function is a way to notify of programmatic closure to avoid the one-frame hole.


        ImGuiTabBarFlags tab_bar_flags = (opt_fitting_flags) | (opt_reorderable ? ImGuiTabBarFlags_Reorderable : 0);
        bool redock_all = false;

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            NotifyOfDocumentsClosedElsewhere();

            // Create a DockSpace node where any window can be docked
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id);

            // Create Windows
            for (std::map<std::string, rt3gui::Document*>::iterator doc = documents.begin(); doc!=documents.end();)
            {
                if (doc->second->Open) {
                    ImGui::SetNextWindowDockID(dockspace_id, redock_all ? ImGuiCond_Always : ImGuiCond_FirstUseEver);
                    ImGuiWindowFlags window_flags = (doc->second->getDirty() ? ImGuiWindowFlags_UnsavedDocument : 0);
                    bool visible = ImGui::Begin(doc->second->getName(), &doc->second->Open, window_flags);

                    if (visible) {
                        doc->second->makeEditorVisible();
                        if (ImGui::IsWindowFocused()) {
                            activeEditor = doc->first;
                        }
                    }
                    else {
                        doc->second->makeEditorInvisible();
                    }
                    doc->second->DisplayContents(toolstrip, activeEditor);

                    ImGui::End();
                    ++doc;
                }
                else {
                    // Cancel attempt to close when unsaved add to save queue so we can display a popup.
                    if (doc->second->getDirty())
                    {
                        doc->second->DoOpen();
                        doc->second->DoQueueClose();
                        ++doc;
                    }
                    else {
                        if (activeEditor == doc->first) {
                            std::string previousActive = activeEditor;
                            if (documents.size() > 1) {
                                activeEditor = std::find_if(documents.begin(), documents.end(), [previousActive](auto& _doc) { return _doc.first != previousActive; })->first;
                            }
                            else {
                                activeEditor = ""; //Если закрывается последний редактор, то в активный редактор записываем пустую строку
                            }
                        }
                        doc = documents.erase(doc); //Возвращаем итератор на редактор, который будет вместо удаленного 
                        
                    }
                }

            }
        }


    // Update closing queue
    static std::vector<std::pair<std::string, rt3gui::Document*>> close_queue;
    if (close_queue.empty())
    {
        // Close queue is locked once we started a popup
        for (auto& doc : documents)
        {
            if (doc.second->getWantClose())
            {
                doc.second->setWantClose(false);
                close_queue.push_back(doc);
            }
        }
    }

    // Display closing confirmation UI
    if (!close_queue.empty())
    {
        int close_queue_unsaved_documents = 0;
        for (int n = 0; n < close_queue.size(); n++)
            if (close_queue[n].second->getDirty())
                close_queue_unsaved_documents++;

        if (close_queue_unsaved_documents == 0)
        {
            // Close documents when all are unsaved
            for (int n = 0; n < close_queue.size(); n++)
                File_Close();
            close_queue.clear();
        }
        else
        {
            if (!ImGui::IsPopupOpen("Save?"))
                ImGui::OpenPopup("Save?");
            if (ImGui::BeginPopupModal("Save?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Save change to the following items?");
                float item_height = ImGui::GetTextLineHeightWithSpacing();
                if (ImGui::BeginChildFrame(ImGui::GetID("frame"), ImVec2(-FLT_MIN, 6.25f * item_height)))
                {
                    for (int n = 0; n < close_queue.size(); n++)
                        if (close_queue[n].second->getDirty())
                            ImGui::Text("%s", close_queue[n].first.c_str());
                    ImGui::EndChildFrame();
                }

                ImVec2 button_size(ImGui::GetFontSize() * 7.0f, 0.0f);
                if (ImGui::Button("Yes", button_size))
                {
                    for (int n = 0; n < close_queue.size(); n++)
                    {
                        if (close_queue[n].second->getDirty())
                            File_Save(close_queue[n].first);
                        File_Close(close_queue[n].first);
                    }
                    close_queue.clear();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("No", button_size))
                {
                    for (int n = 0; n < close_queue.size(); n++)
                        File_Close();
                    close_queue.clear();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", button_size))
                {
                    close_queue.clear();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
    }
}

void Rt3Editor::UpdateActions()
{
    bool hasDocument = documents.size();
    auto isModified = hasDocument && File_IsModified();

    auto hasUndo = hasDocument && !documents[activeEditor]->m_Undo.empty();
    auto hasRedo = hasDocument && !documents[activeEditor]->m_Redo.empty();
    if (hasDocument) {
        toolstrip->setPauseState(false);
        toolstrip->setStartState(false);
    }
    else {
        toolstrip->setPauseState(false);
        toolstrip->setStartState(false);
    }

    toolstripActions["Close"].SetEnabled(hasDocument);
    toolstripActions["SaveAs"].SetEnabled(hasDocument);
    toolstripActions["Save"].SetEnabled(isModified);

    toolstripActions["Undo"].SetEnabled(hasUndo && !toolstrip->getStartState());
    toolstripActions["Redo"].SetEnabled(hasRedo && !toolstrip->getStartState());

    toolstripActions["Start"].SetEnabled(hasDocument && !toolstrip->getStartState());
    toolstripActions["Stop"].SetEnabled(hasDocument && toolstrip->getStartState());
    toolstripActions["Pause"].SetEnabled(hasDocument && toolstrip->getStartState() && !toolstrip->getPauseState());
    toolstripActions["Unpause"].SetEnabled(hasDocument && toolstrip->getStartState() && toolstrip->getPauseState());
    toolstripActions["Step"].SetEnabled(hasDocument && (toolstrip->getPauseState() || !toolstrip->getStartState()));

    toolstripActions["Cut"].SetEnabled(hasDocument && false);
    toolstripActions["Copy"].SetEnabled(hasDocument && false);
    toolstripActions["Paste"].SetEnabled(hasDocument && false);
    toolstripActions["Duplicate"].SetEnabled(hasDocument && false);
    toolstripActions["Delete"].SetEnabled(hasDocument && false);
    toolstripActions["SelectAll"].SetEnabled(hasDocument && false);

    toolstripActions["NavigateBackward"].SetEnabled(hasDocument && false);
    toolstripActions["NavigateForward"].SetEnabled(hasDocument && false);

    toolstripActions["Settings"].SetEnabled(hasDocument && !toolstrip->getStartState());
    
    //auto entryNode = m_Blueprint ? FindEntryPointNode(*m_Blueprint) : nullptr;

    //bool hasBlueprint = (m_Blueprint != nullptr);
    //bool hasEntryPoint = (entryNode != nullptr);
    //bool isExecuting = hasBlueprint && (m_Blueprint->CurrentNode() != nullptr);

    //m_Blueprint_Start.SetEnabled(hasBlueprint && hasEntryPoint);
    //m_Blueprint_Step.SetEnabled(hasBlueprint && isExecuting);
    //m_Blueprint_Stop.SetEnabled(hasBlueprint && isExecuting);
    //m_Blueprint_Run.SetEnabled(hasBlueprint && hasEntryPoint);
}

void Rt3Editor::Application_Frame()
{

    auto& io = ImGui::GetIO();

    ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

    //auto& style = ImGui::GetStyle();

# if 0
    {
        for (auto x = -io.DisplaySize.y; x < io.DisplaySize.x; x += 10.0f)
        {
            ImGui::GetWindowDrawList()->AddLine(ImVec2(x, 0), ImVec2(x + io.DisplaySize.y, io.DisplaySize.y),
                IM_COL32(255, 255, 0, 255));
        }
    }
# endif


    static float leftPaneWidth = 400.0f;
    static float rightPaneWidth = 800.0f;
    Splitter(true, 4.0f, &leftPaneWidth, &rightPaneWidth, 50.0f, 50.0f);

    //ShowLeftPane(leftPaneWidth - 4.0f);
    //ImGui::SameLine(0.0f, 12.0f);

    static bool use_work_area = true;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar;

    // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
    // Based on your use case you may want one of the other.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
    ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

    if (!ImGui::Begin("Editor", NULL, flags)) {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }
    else{
        
        DrawMenuBar();

        toolstrip->setCurrentSamplesNum(0);
        toolstrip->drawToolstrip();
        devicesTree.DrawNodesTree();
        ImGui::SameLine();

        UpdateActions();
        ShowDocumentsApp(NULL);

        if (showSettingsWindow) {
        }
    }
    ImGui::End();

    //ImGui::ShowTestWindow();
    //ImGui::ShowMetricsWindow();
}


void Rt3Editor::UpdateTitle() {
    std::string title = "RT3 Editor";

    if (documents.size())
    {
        title += " - ";
        title += documents[activeEditor]->m_Name.c_str();

        if (File_IsModified())
            title += "*";
    }

    SetMainWindowTitle(title.c_str());
}