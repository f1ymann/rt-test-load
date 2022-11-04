#ifndef  RT3GUI_NODE_H
#define RT3GUI_NODE_H

#include <string>
#include <vector>
#include <imgui_node_editor.h>
#include <crude_json.h>

namespace ed = ax::NodeEditor;

namespace rt3gui {

    enum SampleTypes {
        ComplexFloat,
        FloatPoint,
        Integer
    };
    // Struct to hold basic information about connection between
    // pins. Note that connection (aka. link) has its own ID.
    // This is useful later with dealing with selections, deletion
    // or other operations.
    struct LinkInfo
    {
        ed::LinkId Id;
        ed::PinId  InputId;
        ed::PinId  OutputId;
    };

    struct Link
    {
        ed::LinkId ID;

        ed::PinId StartPinID;
        ed::PinId EndPinID;

        ImColor Color;

        Link(const crude_json::value& value) {
            uint32_t val;
            detail::GetTo<crude_json::number>(value, "id", val);
            ID = ed::LinkId(val);
            detail::GetTo<crude_json::number>(value, "start_id", val);
            StartPinID = ed::PinId(val);
            detail::GetTo<crude_json::number>(value, "end_id", val);
            EndPinID = ed::PinId(val);
            detail::GetTo<crude_json::number>(value, "color", val);
            Color = ImColor(val);
        }

        Link(ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId) :
            ID(id), StartPinID(startPinId), EndPinID(endPinId), Color(255, 255, 255)
        {
        }
    };

    struct NodeIdLess
    {
        bool operator()(const ed::NodeId& lhs, const ed::NodeId& rhs) const
        {
            return lhs.AsPointer() < rhs.AsPointer();
        }
    };

    enum class PinKind
    {
        Output,
        Input
    };

    enum class NodeType
    {
        Blueprint,
        Simple,
        Tree,
        Comment,
        Houdini
    };
    inline const char* ToString(NodeType v)
    {
        switch (v)
        {
        case NodeType::Blueprint:   return "Blueprint";
        case NodeType::Simple:   return "Simple";
        case NodeType::Tree: return "Tree";
        case NodeType::Comment: return "Comment";
        case NodeType::Houdini: return "Houdini";
        default:      return "[Unknown Node type]";
        }
    }

    struct Node;

    struct Pin
    {
        ed::PinId   ID;
        rt3gui::Node* Node;
        std::string Name;
        rt3gui::SampleTypes Type;
        PinKind     Kind;

        Pin() {
            Node = nullptr;
        }

        Pin(int id, const char* name, rt3gui::SampleTypes type) :
            ID(id), Node(nullptr), Name(name), Type(type), Kind(PinKind::Input)
        {
        }
        Pin(const Pin& _pin) :ID(_pin.ID), Node(_pin.Node), Name(_pin.Name), Type(_pin.Type), Kind(_pin.Kind) {}

        Pin& operator=(const Pin& _pin) {
            if (this != &_pin) {
                ID = _pin.ID;
                Node = _pin.Node;
                Name = _pin.Name;
                Type = _pin.Type;
                Kind = _pin.Kind;
            }
            return *this;
        }

        bool Load(const crude_json::value& value)
        {
            uint32_t val;
            if (!detail::GetTo<crude_json::number>(value, "id", val)) { // required
                return false;
            }
            ID = ed::PinId(val);
            if (!detail::GetTo<crude_json::number>(value, "sample_type", Type)) { // required
                return false;
            }
            if (!detail::GetTo<crude_json::number>(value, "kind", Kind)) { // required
                return false;
            }

            if (detail::GetTo<crude_json::string>(value, "name", Name)) {
            } // optional
            return true;
        }

        void Save(crude_json::value& value) const
        {
            value["id"] = crude_json::number(ID.Get()); // required
            value["sample_type"] = crude_json::number(Type); // required
            value["kind"] = crude_json::number(Kind); // required

            if (!Name.empty())
                value["name"] = Name;  // optional, to make data readable for humans
        }
    };


    struct Node
    {
        ed::NodeId ID;
        std::string Name;
        std::vector<Pin> Inputs;
        std::vector<Pin> Outputs;
        ImColor Color;
        NodeType Type;
        ImVec2 Size;
        uint32_t devId;

        std::string State;
        std::string SavedState;

        Node() {

        }

        Node(int id, const char* name, ImColor color = ImColor(255, 255, 255)) :
            ID(id), Name(name), Color(color), Type(NodeType::Blueprint), Size(0, 0)
        {
        }

        bool Load(const crude_json::value& value)
        {
            if (!value.is_object())
                return false;
            
            if (!detail::GetTo<crude_json::number>(value, "id", ID)) // required
                return false;
            if (!detail::GetTo<crude_json::number>(value, "color", Color))
                return false;
            if (!detail::GetTo<crude_json::string>(value, "name", Name))
                return false;

            const crude_json::array* inputPinsArray = nullptr;
            if (detail::GetPtrTo(value, "input_pins", inputPinsArray)) // optional
            {
                Inputs.resize(inputPinsArray->size());

                for (size_t i = 0; i < inputPinsArray->size(); ++i)
                {
                    if (!Inputs[i].Load(inputPinsArray[0][i]))
                        return false;
                    Inputs[i].Node = this;
                }
            }

            const crude_json::array* outputPinsArray = nullptr;
            if (detail::GetPtrTo(value, "output_pins", outputPinsArray)) // optional
            {
                Outputs.resize(outputPinsArray->size());

                for (size_t i = 0; i < outputPinsArray->size(); ++i)
                {
                    if (!Outputs[i].Load(outputPinsArray[0][i]))
                        return false;
                    Outputs[i].Node = this;
                }
            }

            return true;
        }

        void Save(crude_json::value& value) const
        {
            value["id"] = crude_json::number(ID.Get()); // required
            value["color"] = crude_json::number(Color);
            value["name"] = Name; // optional, to make data readable for humans

           
            auto& inputPinsValue = value["input_pins"]; // optional
            for (auto& pin : const_cast<Node*>(this)->Inputs)
            {
                crude_json::value pinValue;
                pin.Save(pinValue);
                inputPinsValue.push_back(pinValue);
            }
            if (inputPinsValue.is_null())
                value.erase("input_pins");

            auto& outputPinsValue = value["output_pins"]; // optional
            for (auto& pin : const_cast<Node*>(this)->Outputs)
            {
                crude_json::value pinValue;
                pin.Save(pinValue);
                outputPinsValue.push_back(pinValue);
            }
            if (outputPinsValue.is_null())
                value.erase("output_pins");
        }
    };

} //rt3gui

#endif // ! RT3GUI_NODE_H