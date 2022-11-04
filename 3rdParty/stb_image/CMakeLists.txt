project(stb_image)

add_library(${PROJECT_NAME} INTERFACE)
add_library(3rdParty::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set(_stb_image_Sources
    ${CMAKE_CURRENT_SOURCE_DIR}/stb_image.h
)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(${PROJECT_NAME} INTERFACE ${_stb_image_Sources})