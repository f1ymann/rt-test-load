cmake_minimum_required(VERSION 3.7.2)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(UNIX)
    #set(CMAKE_CXX_COMPILER  /usr/bin/clang++-11)
    #set(CMAKE_CXX_COMPILER /usr/bin/g++-11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -fpermissive" )
    set(OpenGL_GL_PREFERENCE GLVND)
    link_libraries(stdc++fs)
 
    execute_process(COMMAND ${BASH_EXECUTABLE} ./dependencies.sh
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE BASH_RESULT)

elseif(WIN32)

    if(MSVC)

     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20")
 
    endif(MSVC)

endif()

project(RTCube LANGUAGES CXX)

if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 11)
    set(CMAKE_CXX_STANDARD 20)
else()
    set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

find_package(Threads REQUIRED)

option (BUILD_3RD_PARTY_LIB "Build 3rd party libraries." ON)
option(BUILD_LIB "Build libraries." ON)
option(BUILD_UTILS "Build utils." ON)
option(BUILD_TESTS "Build tests." ON)

IF(BUILD_3RD_PARTY_LIB)

 add_subdirectory(3rdParty)

ENDIF(BUILD_3RD_PARTY_LIB)

IF(BUILD_LIB)

 add_subdirectory(lib)

ENDIF(BUILD_LIB)

IF(BUILD_UTILS)

 add_subdirectory(utils)

ENDIF(BUILD_UTILS)
