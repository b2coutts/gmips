#ifndef PARSE_H
#define PARSE_H

#include <stdint.h>
#include "avl.h"

typedef char type_t;
typedef uint32_t word;

// a single MIPS instruction
// TODO: union?
struct inst {
    // 1-18 mean .word, add, sub, mult, multu, div, divu, mfhi, mflo, lis, lw,
    // sw, slt, sltu, beq, bne, jr, jalr (respectively); 0 means no instruction.
    // -1 means error; an error message will be in .is (see below)
    type_t type;

    // d,s,t registers (as in mipsref.pdf)
    uint8_t d,s,t;

    // two's complement integer argument
    int32_t i;

    // string representing a label used in place of above argument; if lbl ==
    // 0, then i is used; otherwise, lbl is used
    char *lbl;
};

// parse an inst from str. If an error occurs, write an error message to err
struct inst inst_parse(char *str, unsigned int line, char *err,
    long int addr, struct AVLTree *lbls);

// if in has a label, replace it with a number using the symbol table lbls
void lbl_replace(struct AVLTree *lbls, struct inst *in, unsigned int line,
                 long int addr, char *err);

// decode an inst from a 32-bit word
struct inst inst_decode(word w);

// encode an inst into a 32-bit word
word inst_encode(struct inst in);

#endif
