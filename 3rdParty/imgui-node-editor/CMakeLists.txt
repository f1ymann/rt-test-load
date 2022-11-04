#сборка библиотеки imgui-node-editor
project(imgui_node_editor)

set(_imgui_node_editor_Includes
    crude_json.h
	crude_logger.h
    imgui_bezier_math.h
    imgui_canvas.h
    imgui_extra_math.h
    imgui_node_editor_internal.h
    imgui_node_editor.h
	imgui_extras.h
)

set(_imgui_node_editor_Sources
    crude_json.cpp
	crude_logger.cpp
    imgui_bezier_math.inl
    imgui_canvas.cpp
    imgui_extra_math.inl
    imgui_node_editor_api.cpp
    imgui_node_editor_internal.inl
    imgui_node_editor.cpp
	imgui_extras.cpp
)

add_library(${PROJECT_NAME} STATIC ${_imgui_node_editor_Includes} ${_imgui_node_editor_Sources})
add_library(3rdParty::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PUBLIC 3rdParty::imgui)