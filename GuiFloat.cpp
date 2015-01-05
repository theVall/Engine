#include "GuiFloat.h"

GuiFloat::GuiFloat()
{
}


GuiFloat::GuiFloat(const GuiFloat &)
{
}


GuiFloat::~GuiFloat()
{
}


void GuiFloat::Initialize(const char *name)
{

}


void GuiFloat::SetValue(float newValue)
{
    m_value = newValue;
}


float GuiFloat::GetValue()
{
    return m_value;
}