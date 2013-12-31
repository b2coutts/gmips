#ifndef PARSE_H
#define PARSE_H

#include <stdint.h>
#include "avl.h"

typedef char type_t;

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

// TODO: needed?
struct inst inst_create(type_t ty, uint8_t d, uint8_t s, uint8_t t, int i);

// parse an inst from str. If an error occurs, write an error message to err
struct inst inst_parse(char *str, unsigned int line, char *err,
    unsigned int addr, struct AVLTree *lbls);

// decode an inst from a 32-bit word
struct inst inst_decode(uint32_t w);

// encode an inst into a 32-bit word
uint32_t inst_encode(struct inst in);

#endif
