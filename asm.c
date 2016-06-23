/****************************************************************************
 * asm.c
 *
 * Elliott Fiedler, 2016
 * Hack Assembler from assembly (.asm) to machine language (.hack)
 * Unit 6 Project for "Nand to Tetris," Coursera 2016
 *
 * Implements assembly from Hack assembly code  (.asm) to machine language (.hack)
 * written in consideration of the Hack computer specifications:
 * 16-bit instructions
 * A- and C-instructions
 * D and A (M) registers
 * Symbols (pre-defined, labels and user-defined variables)
 ***************************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm.h"

int main(int argc, char* argv[]) {
    
    // check command-line args
    if (argc != 3) {
        printf("Usage: asm [input.asm] [output.hack]\n");
        return 1;
    }
    
    // check file type
    const char* infile = argv[1];     // ".asm"
    const char* outfile = argv[2];    // ".hack"

    if (strrchr(infile, '.') == NULL) {
        printf("Invalid file type. \nUse files with extension \".asm\".\n");
        return 1;
    }

    // check input file extension
    char* fileExt = malloc(strlen(infile) * sizeof(char));
    strcpy(fileExt, strchr(infile, '.'));
    if (strcasecmp(fileExt, ".asm") != 0)
    {
        printf("Invalid file extension. \nUse file extension \".asm\" for input files.\n");
        free(fileExt);
        return 1;
    }
    free(fileExt);

    // check output file extension
    fileExt = malloc(strlen(outfile) * sizeof(char));
    strcpy(fileExt, strchr(outfile, '.'));
    if (strcasecmp(fileExt, ".hack") != 0)
    {
        printf("Invalid file extension. \nUse file extension \".hack\" for output files.\n");
        free(fileExt);
        return 1;
    }
    free(fileExt);

    // open input file (.asm)
    FILE* inptr = fopen(infile, "r");
    if (inptr == NULL) {
        printf("Could not open file \"%s\"\n", infile);
        fclose(inptr);
        return 1;
    }

    // create output file (.hack)
    FILE* outptr = fopen(outfile, "w");
    if (outptr == NULL) {
        printf("Could not create file \"%s\"\n", outfile);
        fclose(outptr);
        return 1;
    }

    // unpack assembly from input file
    bool parsed = parse(inptr);
    if (!parsed) {
        printf("Error parsing input file.\n");
        return 1;
    }

    // convert assembly to binary
    bool decoded = decode(outptr);
    if (!decoded) {
        printf("Error decoding input file.\n");
        return 1;
    }

    // unload assembly from memory
    bool unloaded = unload();
    if (!unloaded) {
        printf("Error unloading memory.\n");
        return 1;
    }

    printf("*** Conversion success! ***\n\n");

    // close input and output files
    fclose(inptr);
    fclose(outptr);
    return 0;
}
