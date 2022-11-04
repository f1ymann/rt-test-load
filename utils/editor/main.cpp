#include <blueprints.h>
#if defined(_WIN32)
#include <Windows.h>
#endif


int main(int argc, char const *argv[])
{
#if defined(_WIN32)
    ShowWindow(GetConsoleWindow(), HIDE_WINDOW); //Спрятать окно
#endif
    Rt3Editor app("Editor Demo",1280,720,argc,argv);
    app.Run();
    return 0;
}
