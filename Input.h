#pragma once

class Input
{
public:

    //  Constructor
    Input(void);
    
    //  Copy Constructor
    Input(const Input&);

    //  Destructor
    ~Input(void);

    //  Initialization
    void Initialize();

    //  Handle pressed keys (set state in key array)
    void ProcessKeyDown(unsigned int);
    
    //  Handle released keys (clear state in key array)
    void ProcessKeyUp(unsigned int);

    //  Get Key state
    bool IsKeyDown(unsigned int);

private:

    //  Member variables
    bool m_keys[256];

};

