// testing driver for parse.c
// TODO: make line numbers unsigned long ints instead of unsigned ints?
#include "parse.h"
#include "avl.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ARRAY_SIZE 1000000

// get the op name from the type_t
char *getop(type_t t){
    if(t == -1) return "ERR";
    if(t == 0) return "NOP";
    if(t == 1) return ".word";
    if(t == 2) return "add";
    if(t == 3) return "sub";
    if(t == 4) return "mult";
    if(t == 5) return "multu";
    if(t == 6) return "div";
    if(t == 7) return "divu";
    if(t == 8) return "mfhi";
    if(t == 9) return "mflo";
    if(t == 10) return "lis";
    if(t == 11) return "lw";
    if(t == 12) return "sw";
    if(t == 13) return "slt";
    if(t == 14) return "sltu";
    if(t == 15) return "beq";
    if(t == 16) return "bne";
    if(t == 17) return "jr";
    if(t == 18) return "jalr";
    return "wat";
}

// remove the comment if present
void remove_comment(char *line){
    while(line[0] != '\0'){
        if(line[0] == ';'){
            line[0] = '\0';
            return;
        }
        line++;
    }
}

// struct for holding instructions and line numbers
struct instline {
    struct inst i;
    long int l;
};

// print out a word as binary
void bin_print(word w){
    putchar(w >> 24);
    putchar((w << 8) >> 24);
    putchar((w << 16) >> 24);
    putchar((w << 24) >> 24);
}

// stop those annoying compiler warnings
size_t getline(char **lineptr, size_t *n, FILE *stream);

int main(int argc, char *argv[]){
    struct instline *code = malloc(sizeof(struct instline) * ARRAY_SIZE);
    long int addr = 0; // keeps track of index in above array
    size_t len = 0; // holds lengths of lines from stdin
    char *line = 0; // holds lines from stdin
    char *err = malloc(100); // holds error messages
    struct AVLTree *lbls = avl_create(); // holds the symbol table
    unsigned int nline = 0; // keeps track of line number from stdin

    if(argc <= 1){
        // TODO: add (optional?) argument for output file
        fprintf(stderr, "Usage: %s [asm_file]\n", argv[0]);
        return 1;
    }
    FILE *fp = fopen(argv[1], "r");
    if(!fp){
        fprintf(stderr, "Could not open file \"%s\".\n", argv[1]);
    }

    // read instructions into array
    while(getline(&line, &len, fp) != -1){
        nline++;
        remove_comment(line);
        struct inst in = inst_parse(line, nline, err, addr, lbls);

        if(in.type == -1){
            fprintf(stderr, "%s", err);
            return 1;
        }else if(in.type != 0){
            code[addr].i = in;
            code[addr].l = nline;
            addr++;
        }
    }

    // replace labels
    for(long int i = 0 ; i < addr; i++){
        lbl_replace(lbls, &code[i].i, code[i].l, i, err);
        if(code[i].i.type == -1){
            fprintf(stderr, "%s", err);
            return 1;
        }
    }

    // print out binary
    for(long int i = 0; i < addr; i++){
        bin_print(inst_encode(code[i].i));
    }
}
