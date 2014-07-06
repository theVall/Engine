#include "Profiler.h"


Profiler::Profiler()
{
}


Profiler::Profiler(const Profiler &)
{
}


Profiler::~Profiler()
{
}


void Profiler::Initialize()
{
    m_fps = 0;
    m_count = 0;
    m_startTime = timeGetTime();
    return;
}


void Profiler::Frame()
{
    m_count++;

    if (timeGetTime() >= (m_startTime + 1000))
    {
        m_fps = m_count;
        m_count = 0;

        m_startTime = timeGetTime();
    }
}


int Profiler::GetFps()
{
    return m_fps;
}