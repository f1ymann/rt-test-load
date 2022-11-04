option(BUILD_UTIL_EDITOR "Build guiEditor." ON)


IF(BUILD_UTIL_EDITOR)

 add_subdirectory (editor)

ENDIF(BUILD_UTIL_EDITOR)
