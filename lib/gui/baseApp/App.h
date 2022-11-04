#ifndef APP_H
#define APP_H

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifdef _WIN32
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#else
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#endif // _WIN32


#include <imgui.h>
//#include <implot.h>
#include <string>
#include <map>

#include "Native.h"
#include "Fonts/Fonts.h"
#include "Helpers.h"

#include "cxxopts.hpp"

/// Macro to disable console on Windows
#if defined(_WIN32) && defined(APP_NO_CONSOLE)
    #pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

// Barebones Application Framework
struct App 
{
    // Constructor.
    App(std::string title, int w, int h, int argc, char const *argv[]);
    App(std::string title, int w, int h, cxxopts::Options options);
    // Destructor.
    virtual ~App();
    // Called at top of run
    virtual void Start() { }
    // Update, called once per frame.
    virtual void Update() { /*implement me*/ }
    // Runs the app.
    void Run();
    // Get window size
    ImVec2 GetWindowSize() const;
    // Set window title
    void SetMainWindowTitle(const char* title);

    ImVec4 ClearColor;                    // background clear color
    std::map<std::string,ImFont*> Fonts;  // font map

#ifdef  _WIN32

    // Data
    ID3D11Device* glob_g_pd3dDevice = NULL;

#endif //  _WIN32


};

#endif //APP_H