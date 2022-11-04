option(BUILD_LIB_APP "Build baseApp." ON)
option(BUILD_LIB_EDITOR "Build editor." ON)

IF(BUILD_LIB_APP)

 add_subdirectory(baseApp)

ENDIF(BUILD_LIB_APP)

IF(BUILD_LIB_EDITOR)

 add_subdirectory(editor)

ENDIF(BUILD_LIB_EDITOR)
