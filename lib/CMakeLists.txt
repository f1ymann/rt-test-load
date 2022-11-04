option(BUILD_LIB_CORE "Build coreLib." ON)
option(BUILD_LIB_GUI "Build guiLib." ON)
option(BUILD_LIB_DEVICES "Build devicesLib." ON)


IF(BUILD_LIB_GUI)

 add_subdirectory(gui)

ENDIF(BUILD_LIB_GUI)