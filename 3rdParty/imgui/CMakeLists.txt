
#сборка библиотеки imgui
project(imgui VERSION 1.88)

if (WIN32)
set(IMGUI_HEADERS
    imconfig.h
    imgui_internal.h
    imgui.h
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h    
    backends/imgui_impl_dx11.h
    backends/imgui_impl_win32.h
    misc/cpp/imgui_stdlib.h
	imgui_stacklayout.h
	imgui_stacklayout_internal.h
)

set(IMGUI_SRC
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui.cpp
    backends/imgui_impl_dx11.cpp
    backends/imgui_impl_win32.cpp
	misc/cpp/imgui_stdlib.h
	imgui_stacklayout.cpp
)
else()

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

set(IMGUI_HEADERS
    imconfig.h
    imgui_internal.h
    imgui.h
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h    
    backends/imgui_impl_glfw.h
    backends/imgui_impl_opengl3.h
    backends/imgui_impl_opengl3_loader.h
    misc/cpp/imgui_stdlib.h
	imgui_stacklayout.h
	imgui_stacklayout_internal.h
)

set(IMGUI_SRC
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui.cpp
    backends/imgui_impl_glfw.cpp
    backends/imgui_impl_opengl3.cpp
    misc/cpp/imgui_stdlib.cpp
	imgui_stacklayout.cpp
)
endif()

find_package(OpenGL REQUIRED)

add_library(imgui ${IMGUI_HEADERS} ${IMGUI_SRC})
add_library(3rdParty::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if(MSVC)
  target_compile_options(imgui PRIVATE /W4 /WX /arch:AVX2 /fp:fast)
endif()
if (WIN32)
  target_link_libraries(imgui PUBLIC d3d11 d3dcompiler dxgi)
else()
  target_link_libraries(imgui PUBLIC ${GLFW_STATIC_LIBRARIES} glad OpenGL::GL)
  target_include_directories(imgui PUBLIC ${GLFW_INCLUDE_DIRS})

endif()

target_compile_definitions(imgui PRIVATE IMGUI_DLL_EXPORT)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/examples ${PROJECT_SOURCE_DIR}/backends ${PROJECT_SOURCE_DIR}/misc/cpp)
