#сборка библиотеки glad
project(glad)
add_library(${PROJECT_NAME} src/glad.c)
add_library(3rdParty::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  
    $<INSTALL_INTERFACE:include>  # <prefix>/include/mylib
)