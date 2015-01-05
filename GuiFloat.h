#pragma once

#include "GUI.h"

class GuiFloat
{
public:
    GuiFloat();
    GuiFloat(const GuiFloat &);
    ~GuiFloat();

    void Initialize(const char *name);

    void SetValue(float newValue);
    float GetValue();

    void SetMinMax(float minValue, float maxValue);
    void SetStep(float step);
    void SetPrecision(float precision);
    void SetReadOnly(bool);

private:
    float m_value;
};
