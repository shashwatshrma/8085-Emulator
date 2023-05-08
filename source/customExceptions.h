#ifndef EXCEPTIONS
#define EXCEPTIONS

#include <iostream>

using namespace std;

class noHLTcommand: exception
{
public:
    string what()
    {
        return "No HLT in the provided code";
    }
};

class invalidCommand: exception
{
    int memLocation;

public:
    invalidCommand(int memLocation)
    {
        memLocation=this->memLocation;
    }

    string what()
    {
        return "Syntax error in the provided code at memory location " + memLocation;
    }
};

class ProgramCounterOutOfBounds: exception
{
public:
    string what()
    {
        return "Memory out of bounds";
    }
};

#endif // EXCEPTIONS
