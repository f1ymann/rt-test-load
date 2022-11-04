option(BUILD_IMGUI "Build imgui." ON)
option(BUILD_IMGUI_NODE_EDITOR "Build imgui node editor." ON)
option(BUILD_GLAD "Build glad." ON)
option(BUILD_NFD "Build native file dialog." ON)
option(BUILD_STB "Build stb image lib." ON)


IF(BUILD_GLAD)
 add_subdirectory(glad)
ENDIF(BUILD_GLAD)

IF(BUILD_IMGUI)
 add_subdirectory(imgui)
ENDIF(BUILD_IMGUI)

IF(BUILD_IMGUI_NODE_EDITOR)
 add_subdirectory(imgui-node-editor)
ENDIF(BUILD_IMGUI_NODE_EDITOR)

IF(BUILD_NFD)
 add_subdirectory(nativefiledialog-extended)
ENDIF(BUILD_NFD)

IF(BUILD_STB)
 add_subdirectory(stb_image)
ENDIF(BUILD_STB)