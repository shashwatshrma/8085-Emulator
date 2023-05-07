# 8085-Emulator
## Features
1. Able to take input from a file or on the command line
2. Contains a Debugger
3. Syntax checking
4. Able to manually input data into registers or memory locations

## Functionalities
- R <register\> - update the value of the specifed register
- M <memory_location\> - update the value at the specified memory location
- G<address\>. - run the program starting at the specified address
- A - update the program
- E - exits simulation

## Supported commands
The Simulator supports the following commands:
- Load and Store -`MOV , MVI , LXI , LDA , STA , LHLD , SHLD , STAX , XCHG`
- Arithmetic - `ADD , ADI , SUB , INR , DCR , INX , DCX , DAD , SUI`
- Logical - `CMA , CMP`
- Branching - `JMP , JC , JNC , JZ , JNZ`
- One Additional command `SET` which sets data into valid memory locations (ex. `SET 2500,0A`)

## Reading input from a file
The path to the input file can be passed as a command line argument when running the executable 

## Using the debugger
Debugger can be used by passing `--debugger` as a command line argument when running the executable

The commands for the debugger are: 
1. break or b <line no\>:- It will set break point on given line number.
2. run or r :- Run the program until it ends or breakpoint is encountered.
3. step or s :- It will run the program one instruction at a time.
4. print or p <register/memory_location\>:- It prints the value of register or memory location. for ex p A will print the value of register A. p x2500 will print the value at memory location x2500 if any.
5. quit or q:- quits the debugger.
6. help:- will show all the commands of debugger.
