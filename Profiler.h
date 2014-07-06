#pragma once

#pragma comment(lib, "winmm.lib")

#include <windows.h>
#include <mmsystem.h>

class Profiler
{
public:
    Profiler();
    Profiler(const Profiler &);
    ~Profiler();

    void Initialize();
    void Frame();
    int GetFps();

private:
    int m_fps;
    int m_count;
    unsigned long m_startTime;
};

