// testing driver for parse.c
// TODO: make line numbers unsigned long ints instead of unsigned ints?
#include "parse.h"
#include "avl.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ARRAY_BUFFER 1000

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

// print a struct inst in a (sort of) human-readable way
void inst_print(struct inst in){
    if(in.lbl == 0){
        printf("%s: $%d, $%d, $%d  [%d]\n",
                getop(in.type), in.d, in.s, in.t, in.i);
    }else{
        printf("%s: $%d, $%d, $%d  [%s]\n",
                getop(in.type), in.d, in.s, in.t, in.lbl);
    }
}

// dump the symbol table to STDOUT
void label_dump(struct node *n){
    if(n){
        label_dump(n->l);
        printf("%s->%ld\n", n->k, n->d);
        label_dump(n->r);
    }
}

// struct for holding instructions and line numbers
struct instline {
    struct inst i;
    long int l;
};

// print out the reversed binary representation of a word
/*
void word_print(word w){
    char *scratch = malloc(50);
    for(int i = 0; i < 32; i++) scratch[i] = '0';

    int ind = 39;
    for(int i = 0; i < 32; i++){
        if(i%4 == 0) scratch[ind--] = ' ';
        scratch[ind--] = w%2 ? '1' : '0';
        w /= 2;
    }

    printf("%s\n", scratch);
    free(scratch);
}
*/

// print out a word as binary
void bin_print(word w){
    putchar(w >> 24);
    putchar((w << 8) >> 24);
    putchar((w << 16) >> 24);
    putchar((w << 24) >> 24);
}

int main(){
    char *asdf = malloc(100000);
    size_t array_size = ARRAY_BUFFER; // size of instruction list (array)
    struct instline *code = malloc(sizeof(struct instline) * array_size);
    long int addr = 0; // keeps track of index in above array
    size_t len = 0; // holds lengths of lines from stdin
    char *line = 0; // holds lines from stdin
    char *err = malloc(100); // holds error messages
    struct AVLTree *lbls = avl_create(); // holds the symbol table
    unsigned int nline = 0; // keeps track of line number from stdin

    // read instructions into array
    while(getline(&line, &len, stdin) != -1){
        nline++;
        remove_comment(line);
        struct inst in = inst_parse(line, nline, err, addr, lbls);

        if(in.type == -1){
            fprintf(stderr, "%s", err);
            return 1;
        }else if(in.type != 0){
            if(addr >= array_size){ // enlarge array if needed
                array_size += ARRAY_BUFFER;
                printf("reallocing to %d\n", array_size);
                code = realloc(code, array_size);
            }
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

    // print stuff
    /*
    printf("\nLabel dump:\n");
    label_dump(lbls->root);
    printf("\n");
    printf("Code dump: (after replacing labels)\n");
    for(long int i = 0; i < addr; i++){
        printf("%08ld: ", i*4);
        inst_print(code[i].i);
    }
    */

    // print out binary
    for(long int i = 0; i < addr; i++){
        //printf("%08ld: ", i*4);
        bin_print(inst_encode(code[i].i));
    }
}
