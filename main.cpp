#include "register.h"
#include "loading.h"
#include "logical.h"
#include "arithmetic.h"
#include "branching.h"

#include <iostream>
#include <string.h>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <bitset>

using namespace std;

void execute(map<int, string> &memory, map<string, register_8bit> &registers, string command, map<int, string>::iterator &PC, bool &jumped);
string parseCommand(string input);

//used when executing the 8085 program
void updateProgramCounter(map<int, string>::iterator &PC, bool &jumped)
{
    if(jumped==true)
    {
        jumped=false;
        return;
    }
    map<string, int> commands= {
        {"MVI", 2}, {"ADI", 2}, {"SUI", 2}, {"LXI", 3}, {"LDA", 3},
        {"STA", 3}, {"LHLD", 3}, {"SHLD", 3}, {"SET", 4}
    };
    auto search = commands.find(parseCommand(PC->second));
    PC++;
    if(search != commands.end())
    {
        for(int i=0; i<search->second-1; i++)
            PC++;
    }
}

//used when inputting the 8085 program into the memory
void updateProgramCounter(map<int, string> &memory, int &PC, string &input)
{
    map<string, int> commands= {
        {"MVI", 2}, {"ADI", 2}, {"SUI", 2}, {"LXI", 3}, {"JMP", 3},
        {"JC", 3}, {"JNC", 3}, {"JZ", 3}, {"JNZ", 3}, {"LDA", 3},
        {"STA", 3}, {"LHLD", 3}, {"SHLD", 3}, {"SET", 4}
    };
    auto search = commands.find(parseCommand(input));
    PC++;
    if(search != commands.end())
    {
        for(int i=0; i<search->second-1; i++)
        {
            memory.insert({PC, " "});
            PC++;
        }
    }
}

//part of the debugger
//runs until a breakpoint or HLT is encountered
void run(map<int, string> &memory, map<string, register_8bit> &registers, map<int, string>::iterator &PC, set<int> &breakpoints, bool &jumped)
{
    while(1)
    {
        if(breakpoints.find(PC->first) != breakpoints.end())
        {
            cout<<"- Break point encountered at "<<PC->first<<"-"<<'\n';
            breakpoints.erase(breakpoints.find(PC->first));
            return;
        }
        else
        {
            if(PC->second=="HLT")
                return;
            execute(memory, registers, PC->second, PC, jumped);
            updateProgramCounter(PC, jumped);
        }
    }
}

//part of the debugger
//fetches single instruction and passes it to execute()
void step(map<int, string>::iterator &PC, map<int, string> &memory, map<string, register_8bit> &registers)
{
    bool jumped=false;
    if(PC->second == "HLT")
        return;
    else
        execute(memory, registers, PC->second, PC, jumped);
    updateProgramCounter(PC, jumped);
}

//part of the debugger
//overloaded function that prints the value of a register
void print(string &reg, map<string, register_8bit> &registers)
{
    cout<<hex<<reg<<": "<<registers[reg].val<<'\n';
}

//part of the debugger
//overloaded function that prints the value of a memory location
void print(int &add, map<int, string> &memory)
{
    cout<<hex<<add<<": "<<memory[add]<<'\n';
}

//part of the debugger
//shows all the debugging options provided to the user
void help()
{
    cout<<"- break/b `line no`:  add a break point at the specified line no."<<'\n';
    cout<<"- run/r - continue execution until the end or next break point"<<'\n';
    cout<<"- step/s - execute 1 instruction"<<'\n';
    cout<<"- print/p `register/xaddress`- print value of register or value at that address"<<'\n';
    cout<<"- quit/q - quit the debugger"<<'\n';
}

//fetches instructions and passes it to execute()
//also checks if debugging is enabled and debus the program
void fetch(map<int, string> &memory, map<string, register_8bit> &registers, int startAddress, bool debug)
{
    string debugOption;
    char switchOption='\0';
    int lineNumber;
    set<int> breakpoints;
    string toPrint;
    int address;
    map<int, string>::iterator PC=memory.find(startAddress);
    if(debug==true)
        cout<<"Debugging"<<endl;
    bool jumped=false;
    while(debug==true)
    {
        cout<<"-> "<<PC->second<<'\n';
        getchar();
        cin>>debugOption;
        if(debugOption == "break" || debugOption == "b")
            switchOption='b';
        else if(debugOption == "run" || debugOption == "r")
            switchOption='r';
        else if(debugOption == "step" || debugOption == "s")
            switchOption='s';
        else if(debugOption == "print" || debugOption == "p")
            switchOption='p';
        else if(debugOption == "quit" || debugOption == "q")
            switchOption='q';
        else if(debugOption == "help")
            switchOption='h';
        switch(switchOption)
        {
        case 'b':
            cin>>hex>>lineNumber;
            breakpoints.insert(lineNumber);
            break;
        case 'r':
            run(memory, registers, PC, breakpoints, jumped);
            break;
        case 's':
            step(PC, memory, registers);
            break;
        case 'p':
            cin>>toPrint;
            if(toPrint[0]=='x')
            {
                stringstream hexAddress;
                hexAddress<<toPrint.substr(1);
                hexAddress>>hex>>address;
                cout<<hex<<address<<": "<<memory[address]<<'\n';
            }
            else
                cout<<hex<<toPrint<<": "<<registers[toPrint].val<<'\n';
            break;
        case 'q':
            debug=false;
            break;
        case 'h':
            help();
            break;
        default:
            cout<<"Invalid debugger command (suggested: use help)"<<'\n';
        }
    }
    while(1)
    {
        if(PC->second == "HLT")
            return;
        else
            execute(memory, registers, PC->second, PC, jumped);
        updateProgramCounter(PC, jumped);
    }
}

//executes single command
void execute(map<int, string> &memory, map<string, register_8bit> &registers, string command, map<int, string>::iterator &PC, bool &jumped)
{
    map<string, int> opcode=
    {
        {"MOV", 1}, {"MVI", 2}, {"LXI", 3}, {"LDA", 4}, {"STA", 5},
        {"LHLD", 6}, {"SHLD", 7}, {"STAX", 8}, {"XCHG", 9},
        {"ADD", 10}, {"ADI", 11}, {"SUB", 12}, {"INR", 13}, {"DCR", 14}, {"INX", 15},
        {"DCX", 16}, {"DAD", 17}, {"SUI", 18},
        {"CMA", 20}, {"CMP", 21},
        {"JMP", 30}, {"JC", 31}, {"JNC", 32}, {"JZ", 33}, {"JNZ", 34},
        {"SET", 40},
    };
    map<string, string> registerPairs= {{"A","F"}, {"B","C"}, {"D","E"}, {"H","L"}};
    stringstream sstr(command);
    string destinationRegister, sourceRegister;
    int immediateData, immediateAddress;
    string instruction;
    sstr>>instruction;
    char ch;
    switch(opcode[instruction])
    {
    case 1:
    {
        sstr>>ch;
        string chartostring1(1,ch);
        destinationRegister=chartostring1;
        sstr>>ch;
        sstr>>ch;
        string chartostring2(1,ch);
        sourceRegister=chartostring2;
        MOV(registers[destinationRegister], registers[sourceRegister]);
        break;
    }
    case 2:
        sstr>>destinationRegister>>hex>>immediateData;
        MVI(registers[destinationRegister], immediateData);
        break;
    case 3:
        sstr>>destinationRegister;
        sstr>>hex>>immediateData;
        LXI(registers[destinationRegister], registers[registerPairs[destinationRegister]], immediateData);
        break;
    case 4:
        sstr>>hex>>immediateAddress;
        LDA(registers["A"], immediateAddress, memory);
        break;
    case 5:
        sstr>>hex>>immediateAddress;
        STA(registers["A"], immediateAddress, memory);
        break;
    case 6:
        sstr>>hex>>immediateAddress;
        LHLD(registers["H"], registers["L"], immediateAddress, memory);
        break;
    case 7:
        sstr>>hex>>immediateAddress;
        SHLD(registers["H"], registers["L"], immediateAddress, memory);
        break;
    case 8:
        sstr>>destinationRegister;
        STAX(registers["A"], registers[destinationRegister], registers[registerPairs[destinationRegister]], memory);
        break;
    case 9:
        XCHG(registers["H"], registers["L"], registers["D"], registers["E"]);
        break;
    case 10:
        sstr>>sourceRegister;
        ADD(registers["A"], registers[sourceRegister], registers["F"]);
        break;
    case 11:
        sstr>>hex>>immediateData;
        ADI(registers["A"], immediateData, registers["F"]);
        break;
    case 12:
        sstr>>sourceRegister;
        SUB(registers["A"], registers[sourceRegister], registers["F"]);
        break;
    case 13:
        sstr>>sourceRegister;
        INR(registers[sourceRegister], registers["F"]);
        break;
    case 14:
        sstr>>sourceRegister;
        DCR(registers[sourceRegister], registers["F"]);
        break;
    case 15:
        sstr>>sourceRegister;
        INX(registers[sourceRegister], registers[registerPairs[sourceRegister]]);
        break;
    case 16:
        sstr>>sourceRegister;
        DCX(registers[sourceRegister], registers[registerPairs[sourceRegister]]);
        break;
    case 17:
        sstr>>sourceRegister;
        DAD(registers["H"], registers["L"], registers[sourceRegister], registers[registerPairs[sourceRegister]], registers["F"]);
        break;
    case 18:
        sstr>>hex>>immediateData;
        SUI(registers["A"], immediateData, registers["F"]);
        break;
    case 20:
        CMA(registers["A"]);
        break;
    case 21:
        sstr>>sourceRegister;
        CMP(registers["A"], registers[sourceRegister], registers["F"]);
        break;
    case 30:
        sstr>>hex>>immediateAddress;
        JMP(memory, PC, immediateAddress);
        jumped=true;
        break;
    case 31:
        sstr>>hex>>immediateAddress;
        JC(memory, PC, immediateAddress, registers["F"]);
        jumped=true;
        break;
    case 32:
        sstr>>hex>>immediateAddress;
        JNC(memory, PC, immediateAddress, registers["F"]);
        jumped=true;
        break;
    case 33:
        sstr>>hex>>immediateAddress;
        JZ(memory, PC, immediateAddress, registers["F"]);
        jumped=true;
        break;
    case 34:
        sstr>>hex>>immediateAddress;
        JNZ(memory, PC, immediateAddress, registers["F"]);
        jumped=true;
        break;
    case 40:
        sstr>>hex>>immediateAddress>>ch>>immediateData;
        SET(immediateAddress, immediateData, memory);
        break;
    }
}

//parses command from the input taken from the user
string parseCommand(string input)
{
    int i;
    for(i=0; input[i]!=0; i++)
        if(input[i]==' ')
            break;
    return input.substr(0, i);
}

//takes input from the user in case no input file is provided
void input(map<int, string> &memory, int PC)
{
    string input;
    while(1)
    {
        map<string, int> commands= {{"MVI", 2}, {"ADI", 2}, {"SUI", 2}, {"LXI", 3}, {"JMP", 3}, {"JC", 3},
            {"JNC", 3}, {"JZ", 3}, {"JNZ", 3}, {"LDA", 3}, {"STA", 3}, {"LHLD", 3}, {"SHLD", 3}
        };
        cout<<hex<<PC<<" ";
        getline(cin, input);
        if(cin.ios::eof())
            break;
        memory.insert({PC, input});
        updateProgramCounter(memory, PC, input);
    }
}

//takes input from a file and puts it into the memory
void inputFromFile(map<int, string> &memory, fstream &file)
{
    int PC;
    string input;
    while(!file.eof())
    {
        file>>hex>>PC;
        file.ignore(1,' ');
        getline(file, input);
        memory.insert({PC, input});
        updateProgramCounter(memory, PC, input);
    }
}

//overloaded function that displays the value of all registers
void display(map<string, register_8bit> registers)
{
    cout<<"Registers:"<<'\n';
    for(auto it:registers)
    {
        if(it.first == "F")
            continue;
        cout<< it.first << ": " << hex << (it.second.val & 0xff) << '\n';
    }
    bitset<8> conditionFlags(registers.find("F")->second.val);
    cout<<'\n'<<"Flags:"<<'\n';
    cout<<"S:  "<<conditionFlags[7] <<'\n';
    cout<<"Z:  "<<conditionFlags[6]<<'\n';
    cout<<"AC: "<<conditionFlags[4]<<'\n';
    cout<<"P:  "<<conditionFlags[2]<<'\n';
    cout<<"CY: "<<conditionFlags[0]<<'\n';
}

//overloaded function that displays all the memory locations used
void display(map<int, string> &memory)
{
    cout<<"Memory used:"<<'\n';
    for(auto it:memory)
        if(it.second != " ")
            cout<<hex<<it.first<<" "<<it.second<<'\n';
}

int main(int argc, char **argv)
{
    map<int, string> memory;
    bool debug=false, fileProvided=false;
    for(int i=1; i<argc; i++)
    {
        if(strcmp(argv[i], "--debugger")==0)
            debug=true;
        else
        {
            cout<<"Reading input from "<<argv[i]<<'\n';
            fstream file;
            file.open(argv[i]);
            inputFromFile(memory, file);
            fileProvided=true;
        }
    }
    register_8bit A={0}, F={0}, B={0}, C={0}, D={0}, E={0}, H={0}, L={0};
    map<string, register_8bit> registers={{"A",A},{"F",F},{"B",B},{"C",C},{"D",D},{"E",E},{"H",H},{"L",L}};
    int startAddress;
    string debugOption;
    if(fileProvided==false)
    {
        cout<<"No input file detected, please enter an 8085 program"<<'\n'<<'\n';
        cout<<"Starting address: ";
        cin>>hex>>startAddress;
        cout<<'\n';
        getchar();
        input(memory, startAddress);
    }
    char ch;
    string reg;
    int address, data;
    char choice;
    cin.clear();
    bool executed=false;
    while(executed==false)
    {
        cin>>choice;
        switch(choice)
        {
        case 'R':
            cin>>reg;
            cout<<reg<<": "<<registers[reg].val<<" - ";
            cin>>data;
            registers[reg].val=data;
            break;
        case 'M':
            cin>>hex>>address;
            cout<<hex<<address<<": "<<memory[address]<<" - ";
            cin>>data;
            memory[address]=itos(data);
            break;
        case 'G':
            cin>>hex>>startAddress;
            cin>>ch;
            if(ch == '.')
            {
                fetch(memory, registers, startAddress, debug);
                executed=true;
                cout<<'\n';
                display(registers);
                cout<<'\n';
                display(memory);
            }
            else
                cout<<"ERROR"<<'\n';
            break;
        default:
            cout<<"ERROR"<<'\n';
        }
    }
    return 0;
}