#ifndef RT3GUI_EDITOR_H
#define RT3GUI_EDITOR_H

#include <string>
#include <vector>
#include <map>

#include <imgui_node_editor.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <imgui_extras.h>
#include <crude_json.h>
#include <crude_logger.h>

#include <toolstrip.h>
#include <node.h>
#include <texturesLoader.h>
#include "utilities/widgets.h"
#include "utilities/drawing.h"
#include "utilities/builders.h"

namespace ed = ax::NodeEditor;
namespace util = ax::NodeEditor::Utilities;
using namespace ax;
using ax::Widgets::IconType;

namespace rt3gui {

    //forward declaration for UndoTransaction
    struct Document;
	
    struct IdGenerator
    {
        uint32_t GenerateId();

        void SetState(uint32_t state);
        uint32_t State() const;

    private:
        uint32_t m_State = 1;
    };
    
	class Editor {
	public:
		Editor(const char* id, std::shared_ptr < TexturesLoader> _textureLoader, ed::Config& config, Document* _document);

		~Editor();

		void drawEditor();

        bool Load(const crude_json::value& value, ed::Config& config);
        bool Load(std::string_view path, ed::Config& config);

        void Save(crude_json::value& value) const;
        bool Save(std::string_view path) const;

        bool startScheme();

        bool pauseScheme();

        bool stepScheme();

        bool stopScheme();

        void setCurrentEditor() { ed::SetCurrentEditor(m_Editor); }

        void DestroyEditor() { ed::DestroyEditor(m_Editor); m_Editor = nullptr; }

        void ShowStyleEditor(bool* show = nullptr);

        bool isVisible();

        void makeVisible();

        void makeInvisible();

        const Node* FindNode(ed::NodeId id) const;
        void TouchNode(ed::NodeId id);


	private:

        ed::LinkId GetNextLinkId();
        float GetTouchProgress(ed::NodeId id);
        void UpdateTouch();
        Link* FindLink(ed::LinkId id);
        Pin* FindPin(ed::PinId id);
        bool IsPinLinked(ed::PinId id);
        bool CanCreateLink(Pin* a, Pin* b);
        void BuildNode(Node* node);
        void BuildNodes();
        ImColor GetIconColor(rt3gui::SampleTypes type);
        void DrawPinIcon(const Pin& pin, bool connected, int alpha);

        Node* CreateNode(uint32_t deviceId);

        Node* SpawnInputActionNode();
        Node* SpawnBranchNode();
        Node* SpawnDoNNode();
        Node* SpawnOutputActionNode();
        Node* SpawnSetTimerNode();
        Node* SpawnTreeSequenceNode();
        Node* SpawnTreeTaskNode();
        Node* SpawnTreeTask2Node();
        Node* SpawnComment();
        Node* SpawnLessNode();
        Node* SpawnWeirdNode();
        Node* SpawnMessageNode();
        Node* SpawnPrintStringNode();
        Node* SpawnHoudiniTransformNode();
        Node* SpawnHoudiniGroupNode();
        Node* SpawnTraceByChannelNode();



		bool                 g_FirstFrame;   // Flag set for first frame only, some action need to be executed once.
		ImVector<LinkInfo>   g_Links;        // List of live links. It is dynamic unless you want to create read-only view over nodes.
		int                  g_NextLinkId;   // Counter to help generate link ids. In real application this will probably based on pointer to user data structure.
		ed::EditorContext* m_Editor;         // Editor context, required to trace a editor state.
        bool visible;

        const int            s_PinIconSize = 24;
        std::vector<Node>    s_Nodes;
        std::vector< std::unique_ptr<Link>>    s_Links;
        ImTextureID          s_HeaderBackground;
        float          s_TouchTime;
        std::map<ed::NodeId, float, NodeIdLess> s_NodeTouchTime;
        IdGenerator                 m_Generator;
        bool createNewNode;
        Pin* newNodeLinkPin;
        Pin* newLinkPin;
        ed::NodeId contextNodeId;
        ed::LinkId contextLinkId;
        ed::PinId  contextPinId;

        std::string id; // ID редактора
        std::shared_ptr<TexturesLoader> textureLoader;

        Document* document;
        bool restoreNodes;
    };

} //rt3gui


#endif //RT3GUI_EDITOR_H