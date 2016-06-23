/****************************************************************************
 * asm.h
 *
 * Elliott Fiedler, 2016
 * Hack Assembler from assembly (.asm) to machine language (.hack)
 * Unit 6 Project
 *
 ***************************************************************************/

#ifndef ASM_H
#define ASM_H

#include <stdbool.h>

// max line length per .asm line
#define LENGTH 1024

// line length of a single assembly command
#define COMMAND 64

// length of a single instruction
#define INSTRUCTION 16

// number of 16-bit words (memory addresses)
#define SIZE 24576

// program table
typedef struct node {
    char word[COMMAND+1];
    struct node* next;
    int line;
} node;

// symbol table (use SIZE here)

// parse each assembly command from infile
bool parse(FILE* infile);

// convert assembly to binary instructions
bool decode(FILE* outfile);

// unload command list from memory
bool unload(void);

// convert decimal to binary:
// http://stackoverflow.com/questions/7911651/decimal-to-binary
void binary(int num, char* string);

#endif // ASM_H