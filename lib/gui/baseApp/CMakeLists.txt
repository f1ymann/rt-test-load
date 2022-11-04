#сборка библиотеки baseApp
project(app)

set (app_HEADERS
  App.h
  Shader.h
  Native.h
  #Image.h
  Fonts/Fonts.h
  cxxopts.hpp
   )

if(WIN32)
set (app_SOURCES
  AppDX11.cpp
  Native.cpp
  Fonts/FontAwesome5.cpp
  Fonts/RobotoRegular.cpp
  Fonts/RobotoBold.cpp
  Fonts/RobotoItalic.cpp
  Fonts/RobotoMonoRegular.cpp
  Fonts/RobotoMonoBold.cpp
  Fonts/RobotoMonoItalic.cpp
   )
else()
set (app_SOURCES
  AppGLFW.cpp
  Native.cpp
  Fonts/FontAwesome5.cpp
  Fonts/RobotoRegular.cpp
  Fonts/RobotoBold.cpp
  Fonts/RobotoItalic.cpp
  Fonts/RobotoMonoRegular.cpp
  Fonts/RobotoMonoBold.cpp
  Fonts/RobotoMonoItalic.cpp
   )
endif() 
 
add_library(${PROJECT_NAME} ${app_HEADERS} ${app_SOURCES} )
add_library(rt3::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} 3rdParty::imgui 3rdParty::nfd)
		     
target_include_directories( ${PROJECT_NAME}
	PUBLIC ${PROJECT_SOURCE_DIR}
)
