/****************************************************************************
 * parse.c
 *
 * Elliott Fiedler, 2016
 * Hack Assembler from assembly (.asm) to machine language (.hack)
 * Unit 6 Project
 *
 * Reads assembly language and translates to 16-bit binary instructions
 * dest = comp; jump (C-Instructions)
 * address in binary (A-Instructions)
 *   
 ***************************************************************************/

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm.h"

// list of assembly commands from infile
node* head;
node* insert_node;
unsigned int count;

// list of label declarations
node* label_head;
node* label_insert;
unsigned int label_count;

// a single assembly command
char command[LENGTH+1];

// symbol table
char* symbol[SIZE+1];

bool parse(FILE* infile)
{
    char buffer[LENGTH+1];  // buffer string
    
    count = 0;
    
    node* new_node = malloc(sizeof(node));
    if (new_node == NULL) {
        printf("Could not allocate memory for new node.\n");
        exit(1);
    }

    printf("\nInput file assembly source:\n\n");

    while (fgets(buffer, LENGTH, infile) != NULL) {
        
        int len = 0; // string length

        // ignore strings beginning with comments or newline
        if ((buffer[0] == '/' && buffer[1] == '/') || (buffer[0] == '\r' || buffer[0] == '\n' || buffer[0] == '\0')) {
            count--;
        }
        // string is indented
        else if (buffer[0] == ' ' || buffer[0] == '\t') {

            // ignore identation
            int i = 0;
            while (buffer[i] == ' ') {
                i++;
            }
            // ignore indented comments
            if (buffer[i] == '/' && buffer[i+1] == '/')
                count--;
            // store command into buffer
            else {
                // get length of string, ignoring whitespace and comments
                int c = 0;
                while (buffer[i] != '\0' && buffer[i] != ' ' && buffer[i] != '/' && buffer[i] != '\r' && buffer[i] != '\n') {
                    len++;
                    command[c] = buffer[i]; // copy buffer to command
                    i++;
                    c++;
                }
            }
        }
        // otherwise
        else {        
            // get length of string
            int i = 0;
            len = 0;
            int c = 0;
            while (buffer[i] != '\0' && buffer[i] != ' ' && buffer[i] != '/' && buffer[i] != '\r' && buffer[i] != '\n') {
                len++;
                command[c] = buffer[i];
                i++;
                c++;
            }
        }
        
        // store assembly commands into linked list
        if (len != 0) {
            
            // store command buffer to new_node
            strcpy(new_node->word, command);
            
            printf("%s\n", command);
            
            new_node->line = count;

            // list is empty
            if (head == NULL) {
                head = malloc(sizeof(node));
                if (head == NULL) {
                    printf("Couldn't allocate memory for head node.\n");
                    return false;
                }
                // copy word and line number from new_node to list
                strcpy(head->word, new_node->word);
                head->line = new_node->line;
                head->next = NULL;
            }
            else {
                // begin traversing from head of list
                node* cursor = head;
                while (cursor->next != NULL) {
                    cursor = cursor->next;
                }
                // insert new node at end of list
                insert_node = malloc(sizeof(node));
                if (insert_node == NULL) {
                    printf("Couldn't allocate memory for insert node.\n");
                    return false;
                }
                strcpy(insert_node->word, new_node->word);
                insert_node->line = new_node->line;
                insert_node->next = NULL;
                cursor->next = insert_node;

            }
            // reset buffers
            int x = 0;
            while(buffer[x] != '\0')
            {
                buffer[x] = '\0';
                x++;
            }
            int y = 0;
            while(command[y] != '\0')
            {
                command[y] = '\0';
                y++;
            }            
        }
        count++;
    }
    free(new_node);
    printf("\n(%i assembly commands)\n\n", count);
    return true;
}

// convert assembly commands to 16-bit binary instructions
bool decode(FILE* outfile)
{
    // populate symbol table with pre-defined symbols
    symbol[0] = "SP";
    symbol[1] = "LCL";
    symbol[2] = "ARG";
    symbol[3] = "THIS";
    symbol[4] = "THAT";
    symbol[16384] = "SCREEN";
    symbol[24576] = "KBD";

    // reset command list
    node* cursor = head;
    
    // traverse label list
    node* label_cursor;
    
    // num of labels
    int num = 0;

    // for variables to be stored at symbol[16+]
    int pos = 16;

    label_count = 0;

    // search for (LABEL) declarations
    while (cursor->word != NULL) {

        if (strchr(cursor->word, '(') != NULL) {

            node* new_node = malloc(sizeof(node));
            if (new_node == NULL) {
                printf("Couldn't allocate memory for label's new node\n");
                return false;
            }
            int length = strlen(cursor->word)-2;
            char temp[length];

            temp[length] = '\0';
            strncpy(temp, strchr(cursor->word, '(')+1, strlen(cursor->word)-2);

            // copy word to new node
            strcpy(new_node->word, temp);

            // label list is empty
            if (label_head == NULL) {

                label_head = malloc(sizeof(node));
                if (label_head == NULL) {
                    printf("Couldn't allocate memory for label_head node.\n");
                    return false;
                }
                
                // copy word to list without surrounding parentheticals
                strcpy(label_head->word, new_node->word);
                label_head->line = cursor->line - num;
                label_head->next = NULL;
            }
            else {
                // begin traversing from label_head of list
                label_cursor = label_head;
                
                while (label_cursor->next != NULL) {
                    label_cursor = label_cursor->next;
                }
                // insert new node at end of list
                label_insert = malloc(sizeof(node));
                if (label_insert == NULL) {
                    printf("Couldn't allocate memory for label insert node.\n");
                    return false;
                }
                // copy label
                strcpy(label_insert->word, new_node->word);

                label_insert->line = cursor->line - num;
                label_insert->next = NULL;
                label_cursor->next = label_insert;
            }
            free(new_node);
            num++;
            label_count++;
        }
        // move cursor to next node in list
        cursor = cursor->next;
    }

    // reset cursor
    cursor = head;

    // convert all assembly to binary
    while (cursor->word != NULL) {
        
        // A-Instructions begin with "0" prefix
        char a_instr[1 + 15 + 1] = "0";
        
        // C-Instruction -- "111" prefix
        char c_instr[3 + 13 + 1] = "111";
        
        // C-Instruction field defaults (dest = comp;jump)
        char* dest = "000";     // d1, d2, d3
        char* comp = "0000000"; // a, c1, c2, c3, c4, c5, c6
        char* jump = "000";     // j1, j2, j3

        bool is_a = false;      // is an A-instruction
        bool a_label = false;   // is a Label
        bool a_addr = false;    // is a normal address
        // command is '@value' (A-Instruction, Label or Variable)
        if (strchr(cursor->word, '@') != NULL) {
            
            // digit or symbol name following '@' character
            is_a = true;
            char* c = strchr(cursor->word, '@')+1;

            int decimal;
            char* address = malloc((INSTRUCTION-1) * sizeof(char));
            if (address == NULL) {
                printf("Failed to allocate memory for address.\n");
                return false;
            }

            // parse address (numbers only)
            if (isdigit(*c) != 0) {
                strcpy(command, c);
                decimal = atoi(command);
                a_addr = true;
            }
            // parse a @LABEL or @VARIABLE (symbol)
            else if (a_addr != true) {
                
                // if @LABEL, find match in label list
                
                // reset label list for traversal
                label_cursor = label_head;

                while (label_cursor != NULL) {

                    if (strcmp(label_cursor->word, c) == 0) {
                        decimal = label_cursor->line;
                        a_label = true;
                    }
                    // increment label list
                    label_cursor = label_cursor->next;
                }

                // otherwise handle symbols, pre-defined and user-defined
                if (a_label == false) {
                    //pre-defined
                    if ((strcasecmp(c, "SP") == 0) || (strcasecmp(c, "R0") == 0))
                        decimal = 0;
                    else if ((strcasecmp(c, "LCL") == 0) || (strcasecmp(c, "R1") == 0))
                        decimal = 1;
                    else if ((strcasecmp(c, "ARG") == 0) || (strcasecmp(c, "R2") == 0))
                        decimal = 2;
                    else if ((strcasecmp(c, "THIS") == 0) || (strcasecmp(c, "R3") == 0))
                        decimal = 3;
                    else if ((strcasecmp(c, "THAT") == 0) || (strcasecmp(c, "R4") == 0))
                        decimal = 4;
                    else if (strcasecmp(c, "R5") == 0)
                        decimal = 5;
                    else if (strcasecmp(c, "R6") == 0)
                        decimal = 6;
                    else if (strcasecmp(c, "R7") == 0)
                        decimal = 7;
                    else if (strcasecmp(c, "R8") == 0)
                        decimal = 8;
                    else if (strcasecmp(c, "R9") == 0)
                        decimal = 9;
                    else if (strcasecmp(c, "R10") == 0)
                        decimal = 10;
                    else if (strcasecmp(c, "R11") == 0)
                        decimal = 11;
                    else if (strcasecmp(c, "R12") == 0)
                        decimal = 12;
                    else if (strcasecmp(c, "R13") == 0)
                        decimal = 13;
                    else if (strcasecmp(c, "R14") == 0)
                        decimal = 14;
                    else if (strcasecmp(c, "R15") == 0)
                        decimal = 15;
                    else if (strcasecmp(c, "SCREEN") == 0)
                        decimal = 16384;
                    else if (strcasecmp(c, "KBD") == 0)
                        decimal = 24576;
                    else {
                        // lookup user-defined variable in table
                        bool exists = false;
                        // position in symbol table
                        for (int i = 16; i < SIZE+1; i++) {

                            // already in table
                            if ((symbol[i] != NULL) && (strcasecmp(symbol[i], c) == 0)) {  
                                decimal = i;
                                exists = true;
                            }
                        }
                        // store symbol
                        if (exists == false) {
                            symbol[pos] = c;
                            decimal = pos;
                            pos++;
                        }
                    }
                }
            }
            // convert decimal to 15-bit binary address
            binary(decimal, address);

            // append binary value to instruction prefix
            strcat(a_instr, address);

            // write to file
            fwrite(a_instr, INSTRUCTION, 1, outfile);
            fputc('\n', outfile); // add new-line character
            if (ferror(outfile) != 0) {
                printf("Error writing to file.\n");
                return false;
            }
            free(address);
        }
        // parse LHS of C-instructions (dest)
        else if (strncmp(cursor->word, "M=", 2) == 0)
            dest = "001";
        else if (strncmp(cursor->word, "D=", 2) == 0)
            dest = "010";
        else if (strncmp(cursor->word, "MD=", 3) == 0)
            dest = "011";
        else if (strncmp(cursor->word, "A=", 2) == 0)        
            dest = "100";
        else if (strncmp(cursor->word, "AM=", 3) == 0)
            dest = "101";
        else if (strncmp(cursor->word, "AD=", 3) == 0)
            dest = "110";
        else if (strncmp(cursor->word, "AMD=", 4) == 0)    
            dest = "111";
        
        // if a C-instruction (no jump), decode the comp field
        if ((is_a == false && strchr(cursor->word, '(') == NULL) && (strncmp(cursor->word, "0;", 2) != 0)) {
            
            // compare RHS of '=' and convert to comp instruction
            if (strchr(cursor->word, '=') != NULL) {

                // = 0
                if (strchr(strchr(cursor->word, '=')+1, '0') != NULL)
                    comp = "0101010";
                // = 1
                else if (strncmp(strchr(cursor->word, '=')+1, "1", 1) == 0)
                    comp = "0111111";
                // = -1
                else if (strncmp(strchr(cursor->word, '=')+1, "-1", 2) == 0)
                    comp = "0111010";
                // = D
                else if (strncmp(strchr(cursor->word, '=')+1, "D", 2) == 0)
                    comp = "0001100";
                // a = 0
                // = A
                else if (strncmp(strchr(cursor->word, '=')+1, "A", 2) == 0)
                    comp = "0110000";
                // = !D
                else if (strncmp(strchr(cursor->word, '=')+1, "!D", 2) == 0)
                    comp = "0001101";
                // = !A
                else if (strncmp(strchr(cursor->word, '=')+1, "!A", 2) == 0)
                    comp = "0110001";
                // = -D
                else if (strncmp(strchr(cursor->word, '=')+1, "-D", 2) == 0)
                    comp = "0001111";    
                // = -A
                else if (strncmp(strchr(cursor->word, '=')+1, "-A", 2) == 0)
                    comp = "0110011";
                // = D+1
                else if (strncmp(strchr(cursor->word, '=')+1, "D+1", 3) == 0)
                    comp = "0011111";
                // = A+1
                else if (strncmp(strchr(cursor->word, '=')+1, "A+1", 3) == 0)
                    comp = "0110111";
                // = D-1
                else if (strncmp(strchr(cursor->word, '=')+1, "D-1", 3) == 0)
                    comp = "0001110";
                // = A-1
                else if (strncmp(strchr(cursor->word, '=')+1, "A-1", 3) == 0)
                    comp = "0110010";
                // = D+A
                else if (strncmp(strchr(cursor->word, '=')+1, "D+A", 3) == 0)
                    comp = "0000010";
                // = D-A
                else if (strncmp(strchr(cursor->word, '=')+1, "D-A", 3) == 0)
                    comp = "0010011";
                // = A-D
                else if (strncmp(strchr(cursor->word, '=')+1, "A-D", 3) == 0)
                    comp = "0000111";
                // = D&A
                else if (strncmp(strchr(cursor->word, '=')+1, "D&A", 3) == 0)
                    comp = "0000000";
                // = D|A
                else if (strncmp(strchr(cursor->word, '=')+1, "D|A", 3) == 0)
                    comp = "0010101";
                // a = 1 (A becomes M)
                // = M
                else if (strncmp(strchr(cursor->word, '=')+1, "M", 2) == 0)
                    comp = "1110000";
                // = !M
                else if (strncmp(strchr(cursor->word, '=')+1, "!M", 2) == 0)
                    comp = "1110001";
                // = -M
                else if (strncmp(strchr(cursor->word, '=')+1, "-M", 2) == 0)
                    comp = "1110011";
                // = M+1
                else if (strncmp(strchr(cursor->word, '=')+1, "M+1", 3) == 0)
                    comp = "1110111";
                // = M-1
                else if (strncmp(strchr(cursor->word, '=')+1, "M-1", 3) == 0)
                    comp = "1110010";
                // = D+M
                else if ((strncmp(strchr(cursor->word, '=')+1, "D+M", 3) == 0) || 
                         (strncmp(strchr(cursor->word, '=')+1, "M+D", 3) == 0))
                    comp = "1000010";
                // = D-M
                else if (strncmp(strchr(cursor->word, '=')+1, "D-M", 3) == 0)
                    comp = "1010011";
                // = M-D
                else if (strncmp(strchr(cursor->word, '=')+1, "M-D", 3) == 0)
                    comp = "1000111";
                // = D&M
                else if (strncmp(strchr(cursor->word, '=')+1, "D&M", 3) == 0)
                    comp = "1000000";
                // = D|M
                else if (strncmp(strchr(cursor->word, '=')+1, "D|M", 3) == 0)
                    comp = "1010101";
            }
            // handle D;JGT, D;JLT, etc.
            else if (strncmp(cursor->word, "D;", 2) == 0) {
                // dest = D
                comp = "0001100";
            }

            // handle jump bits (null, JGT, JEQ, JGE, JLT, JNE, JLE, JMP)
            if (strchr(cursor->word, ';') != NULL) {
                
                if (strncmp(strchr(cursor->word, ';')+1, "JGT", 3) == 0)
                    jump = "001";
                else if (strncmp(strchr(cursor->word, ';')+1, "JEQ", 3) == 0)
                    jump = "010";
                else if (strncmp(strchr(cursor->word, ';')+1, "JGE", 3) == 0)
                    jump = "011";
                else if (strncmp(strchr(cursor->word, ';')+1, "JLT", 3) == 0)
                    jump = "100";
                else if (strncmp(strchr(cursor->word, ';')+1, "JNE", 3) == 0)
                    jump = "101";
                else if (strncmp(strchr(cursor->word, ';')+1, "JLE", 3) == 0)
                    jump = "110";
                else if (strncmp(strchr(cursor->word, ';')+1, "JMP", 3) == 0)
                    jump = "111";
            }
            
            // concatenate dest, comp and jump fields
            snprintf(c_instr, 17, "%s%s%s%s", c_instr, comp, dest, jump);
            
            // write C-instruction to file
            fwrite(c_instr, INSTRUCTION, 1, outfile);
            fputc('\n', outfile); // add new-line character
            if (ferror(outfile) != 0) {
                printf("Error writing to file.\n");
                return false;
            }
        }
        // infinite loop command
        else if ((strcmp(dest, "000") == 0) && (strcmp(cursor->word, "0;JMP") == 0)) {
            
            snprintf(c_instr, 17, "%s0101010000111", c_instr);

            // write C-instruction to file
            fwrite(c_instr, INSTRUCTION, 1, outfile);
            fputc('\n', outfile); // add new-line character
            if (ferror(outfile) != 0) {
                printf("Error writing to file.\n");
                return false;
            }
        }
        // move cursor to next node
        cursor = cursor->next;
    }    
    return true;
}

// unload linked list from memory
bool unload(void) {

    node* cursor = head;
    node* label_cursor = label_head;

    unsigned int compare = 0;
    unsigned int label_compare = 0;

    // free instruction list
    while (cursor != NULL) {
        node* temp = cursor;
        cursor = cursor->next;
        free(temp);
        compare++;
    }
    
    // free labels
    while (label_cursor != NULL) {
        node* temp = label_cursor;
        label_cursor = label_cursor->next;
        free(temp);
        label_compare++;
    }

    if (count == compare && label_count == label_compare)
        return true;
    else
        return false;
}

// convert decimal to 15-bit binary string
void binary(int num, char* string)
{
    *(string+15) = '\0';
    int mask = 0x4000 << 1;
    while(mask >>= 1)
        *string++ = !!(mask & num) + '0';
}