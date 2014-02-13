#include "Main.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
    System *system      = NULL;
    bool initSuccess    = false;

    system = new System;
    if (!system)
    {
        return 0;
    }

    initSuccess = system->Initialize();
    if (initSuccess)
    {
        system->Run();
    }

    //  cleanup
    system->Shutdown();
    delete system;
    system = 0;

    return 0;
}


Main::~Main(void)
{
}
