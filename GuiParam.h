#pragma once

#include <AntTweakBar.h>

template<typename T>
class GuiParam
{
public:
    GuiParam()
    {
        m_changed = false;
    }
    ~GuiParam() {}

    // Setter and getter for the actual GUI parameter.
    void SetValue(T val)
    {
        m_value = val;
        m_changed = true;
    }

    T GetValue() const
    {
        return m_value;
    }

    bool IsChanged()
    {
        return m_changed;
    }

    void ChangeProcessed()
    {
        m_changed = false;
    }

    // Callback function called when the variable value of the tweak bar has changed.
    static void TW_CALL SetCB(const void *pValue, void *pClientData)
    {
        static_cast<GuiParam *>(pClientData)->SetValue(*static_cast<const T *>(pValue));
    }

    // Callback function called by the tweak bar to get the value.
    static void TW_CALL GetCB(void *pValue, void *pClientData)
    {
        *static_cast<T *>(pValue) = static_cast<const GuiParam *>(pClientData)->GetValue();
    }


private:
    // Actual value
    T m_value;
    bool m_changed;
};

