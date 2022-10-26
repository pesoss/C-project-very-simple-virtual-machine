# C-project-very-simple-virtual-machine

Your task will be to implement a virtual machine with random memory
access.

The virtual machine will be invoked as follows:

    ./main program.orc

Where program.orc is a file in ORC format (invented for the purpose of this task),
containing the 'machine code' - the program your virtual machine needs to
fulfill In the examples/ directory you have several example programs.

A program has the following structure:

- (char[3]) - the magic word 'ORC'
- (uint32_t) - the ram_size number that specifies how many memory cells there should be
               available to the machine while running the program (*1)
- the rest of the program is a sequence of instructions.

Each instruction is 25 bytes in size. The first byte of each instruction is
its opcode (a number we treat as the 'number' of the instruction in the table
below), and the remaining 24 bytes are instruction operands. All operands are
of type int64_t, and if the instruction uses less than 3 operands,
unused ones are ignored.

The program operates on memory of size ram_size per number of cells, such as each
cell is of type int64_t. The cells are numbered, and we say they have 'addresses':
the first cell is at address 0, the second at address 1, and so on.

Here is a table with all possible instructions (the 'name' column is only informative and
not directly related to the decision):

===============================================================================

opcode | name | operands | description 
--- | --- | --- | --- 
0x00 | nop | - | does nothing
0x95 | set | tar, val | we write the value val in a cell with address tar 
0x5d | load | to, from_ptr | we copy the value from the address cell the <br/>value in the from_ptr cell in <br/>the cell with the address to
0x63 | store | to_ptr, from | we copy the value from the address cell from <br/>in the address cell the value <br/>of the cell to_ptr
0x91 | jmp | idx | we jump to an instruction with the address value of the <br/>cell with address idx and we <br/>continue the implementation <br/>from there (the first <br/>instruction in the program is numbered 0 etc.)
0x25 | sgz | v | if the value of the cell with address v is > 0, <br/>we skip the next instruction
0xAD | add | res, v1, v2 | in the cell with address res we write the sum <br/>of the numbers in cells with addresses v1 and v2
0x33 | mul | res, v1, v2 | in the cell with address res we write the work <br/>of the numbers in cells with addresses v1 and v2
0x04 | div | res, v1, v2 | in the cell with address res we write the private <br/>of the numbers in cells with addresses v1 and v2 <br/>(*2)
0xB5 | mod | res, v1, v2 | in the cell with address res we write the remainder <br/>of the numbers in cells with addresses v1 and v2 <br/>(*2)
0xC1 | out | v | on stdout we output the character with the ascii code number <br/>in the address cell v
0xBF | sleep | v | block execution for so many milliseconds, <br/>what is the number in the cell with address v
                                
===============================================================================

The virtual machine executes the instructions starting from the first instruction
in the program, and after each instruction (except jmp) it goes to the next one.

When we execute the last instruction from the program, the virtual machine
completes its execution successfully. If during execution it happened
error, the virtual machine terminates with failure. Possible errors are 'invalid
address', 'division by 0', 'invalid instruction', etc.

A Makefile is provided to compile your program. You can edit it
to add additional .c files to your code, but you are not allowed to modify
the compiler error flags. Your program should compile successfully
with the Makefile in question.

(*1) - this means we can allocate all the memory at the beginning, which is
       for convenience.
(*2) - divided by the value of the cell with address v2.

P.S.: You can find the problem in Bulgarian in the problem.txt
