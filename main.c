// testing driver for parse.c
#include "parse.h"
#include "avl.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
        printf("%s->%u\n", n->k, n->d);
        label_dump(n->r);
    }
}

int main(){
    size_t len = 0;
    char *line = 0;
    char *err = malloc(100);
    struct AVLTree *lbls = avl_create();
    unsigned int addr = 0; // TODO: long?
    unsigned int nline = 0;

    printf("Code dump:\n");
    while(getline(&line, &len, stdin) != -1){
        nline++;
        remove_comment(line);
        struct inst in = inst_parse(line, nline, err, addr, lbls);
        if(in.type == -1){
            fprintf(stderr, "%s", err);
            return 1;
        }else if(in.type != 0){
            printf("%08d: ", addr);
            inst_print(in);
            addr += 4;
        }
    }

    printf("\nLabel dump:\n");
    label_dump(lbls->root);
}
