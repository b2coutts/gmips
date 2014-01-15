// TODO: right now, the parser ignores all garbage after the ends of lines
#include "parse.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>

// error messaging macros
#define ERR "assembler: ERROR: line %u: "
#define WERR(msg, ...) sprintf(err, ERR msg, line, __VA_ARGS__);
#define SERR(msg) sprintf(err, ERR msg, line);

// true iff the given string is a valid label
// TODO: find the spec and make this conform to it
int valid_label(const char *str){
    if(!isalpha(str[0])) return 0;
    while(str[1] != '\0'){
        if(!isalpha(str[1]) && !isdigit(str[1])) return 0;
        str++;
    }
    return 1;
}

// duplicate a string into a new location in memory
char *strdup(const char *str){
    char *ret = malloc(strlen(str)+1);
    return strcpy(ret, str);
}

// true iff t is in one of the n optional arguments
int isin(int t, int n, ...){
    va_list vl;
    va_start(vl, n);
    for(int i = 0; i < n; i++){
        if(t == va_arg(vl, int)) return 1;
    }
    va_end(vl);
    return 0;
}
    

// true iff the string is entirely whitespace and comment
int isempty(const char *str){
    for(int i = 0; str[i] != '\0'; i++){
        if(!isspace(str[i])) return 0;
    }
    return 1;
}

// return 2^n
long int pwr2(int p){
    if(p == 0) return 1;
    else return 2 * pwr2(p-1);
}

// attempt to parse a signed n-bit integer from the beginning of str
    // on success: return the int; put rest of string in *r if r != 0
    // on failure: set *t = -1, and write an error message to err
int32_t sint_parse(char *str, int n, type_t *t, char **r, unsigned int line,
                   char *err){
    if(!str){
        *t = -1;
        WERR("reached end of line; expected %d-bit signed integer\n", n);
        return 0;
    }

    char *ret;
    long int i = strtol(str, &ret, 0);

    // special case for hex .word input
    int ishex = 0;
    if(str[0] == '0' && str[1] == 'x'){
        //if(i >= pwr2(n-1)) i -= pwr2(n); // convert from unsigned to signed
        ishex = 1;
    }

    if(ret == str){
        *t = -1;
        WERR("failed to parse %d-bit signed int\n", n);
        return 0;
    }else if((!ishex) && (i < -pwr2(n-1) || i >= pwr2(n-1))){
        if(i > 0 && i < pwr2(n)){
            i -= pwr2(n);
        }else{
            *t = -1;
            WERR("%d-bit signed int '%ld' out of range\n", n, i);
            return 0;
        }
    }else if(ishex && (i > pwr2(n) || i < 0)){
        *t = -1;
        WERR("%d-bit unsigned int '%ld' out of range\n", n, i);
        return 0;
    }

    if(r) *r = ret;
    return i;
}

// attempt to parse a register from the beginning of str
    // on success: return the int; put rest of string in *r if r != 0
    // on failure: set *t = -1, and write an error message to err
uint8_t reg_parse(char *str, type_t *t, char **r, unsigned int line, char *err){
    if(!str){
        *t = -1;
        SERR("reached end of line; expected register\n");
        return 0;
    }

    while(isspace(str[0])) str++;
    if(str[0] != '$'){
        *t = -1;
        SERR("expected '$' before register number\n");
        return 0;
    }
    
    char *ret;
    long int i = strtol(str+1, &ret, 10);
    if(ret == str){
        *t = -1;
        SERR("failed to parse register number\n");
        return 0;
    }else if(i < 0 || i > 31){
        *t = -1;
        WERR("register number '%ld' out of range\n", i);
        return 0;
    }

    if(r) *r = ret;
    return i;
}

// return the typecode for the string
type_t gettype(const char *str){
    if(strcmp(str, ".word") == 0) return 1;
    else if(strcmp(str, "add") == 0) return 2;
    else if(strcmp(str, "sub") == 0) return 3;
    else if(strcmp(str, "mult") == 0) return 4;
    else if(strcmp(str, "multu") == 0) return 5;
    else if(strcmp(str, "div") == 0) return 6;
    else if(strcmp(str, "divu") == 0) return 7;
    else if(strcmp(str, "mfhi") == 0) return 8;
    else if(strcmp(str, "mflo") == 0) return 9;
    else if(strcmp(str, "lis") == 0) return 10;
    else if(strcmp(str, "lw") == 0) return 11;
    else if(strcmp(str, "sw") == 0) return 12;
    else if(strcmp(str, "slt") == 0) return 13;
    else if(strcmp(str, "sltu") == 0) return 14;
    else if(strcmp(str, "beq") == 0) return 15;
    else if(strcmp(str, "bne") == 0) return 16;
    else if(strcmp(str, "jr") == 0) return 17;
    else if(strcmp(str, "jalr") == 0) return 18;
    else return -1;
}

struct inst inst_parse(char *str, unsigned int line, char *err,
                       long int addr, struct AVLTree *lbls){
    struct inst in;
    in.lbl = 0;

    // parse any labels from the beginning of the line
    char *typestr = strtok(str, " \t\n");
    while(typestr && typestr[strlen(typestr)-1] == ':'){
        typestr[strlen(typestr)-1] = '\0';
        if(!valid_label(typestr)){
            in.type = -1;
            WERR("invalid label '%s' declared\n", typestr);
            return in;
        }else if(avl_lookup(lbls, typestr)){
            in.type = -1;
            WERR("duplicate label '%s' declared\n", typestr);
            return in;
        }
        avl_insert(lbls, typestr, addr);
        typestr[strlen(typestr)-1] = ':';
        typestr = strtok(0, " \t\n");
    }

    if(!typestr || isempty(typestr)){
        in.type = 0;
        return in;
    }

    in.type = gettype(typestr);
    if(in.type == -1){
        WERR("invalid operation '%s'\n", typestr);
        return in;
    }

    // TODO: does not ensure commas are there right now
    if(in.type == 1){ // .word
        char *num = strtok(0, " \t\n");
        if(num && isalpha(num[0])){ // attempt to parse a label
            if(valid_label(num)){
                in.lbl = strdup(num);
            }else{
                in.type = -1;
                WERR("invalid label '%s' referenced\n", num);
                return in;
            }
        }else{
            in.i = sint_parse(num, 32, &in.type, 0, line, err);
            if(in.type == -1) return in;
        }
    }else if(isin(in.type, 4, 2,3,13,14)){ // add, sub, slt, sltu
        in.d = reg_parse(strtok(0, " \t\n"), &in.type, 0, line, err);
        if(in.type == -1) return in;
        in.s = reg_parse(strtok(0, " ,\t\n"), &in.type, 0, line, err);
        if(in.type == -1) return in;
        in.t = reg_parse(strtok(0, " ,\t\n"), &in.type, 0, line, err);
        if(in.type == -1) return in;
    }else if(isin(in.type, 4, 4,5,6,7)){ // mult, multu, div, divu
        in.s = reg_parse(strtok(0, " \t\n"), &in.type, 0, line, err);
        if(in.type == -1) return in;
        in.t = reg_parse(strtok(0, " ,\t\n"), &in.type, 0, line, err);
        if(in.type == -1) return in;
    }else if(isin(in.type, 3, 8,9,10)){ // mfhi, mflo, lis
        in.d = reg_parse(strtok(0, " \t\n"), &in.type, 0, line, err);
        if(in.type == -1) return in;
    }else if(isin(in.type, 2, 17,18)){ // jr, jalr
        in.s = reg_parse(strtok(0, " \t\n"), &in.type, 0, line, err);
        if(in.type == -1) return in;
    }else if(isin(in.type, 2, 11,12)){ // lw, sw
        in.t = reg_parse(strtok(0, " \t\n"), &in.type, 0, line, err);
        if(in.type == -1) return in;
        char *next = strtok(0, " \t\n");
        in.i = sint_parse(next, 16, &in.type, &next, line, err);
        if(in.type == -1) return in;
        if(next[0] != '('){
            in.type = -1;
            SERR("expected '(' before $s register\n");
            return in;
        }

        in.s = reg_parse(next+1, &in.type, &next, line, err);
        if(in.type == -1) return in;
        if(next[0] != ')'){
            in.type = -1;
            SERR("expected ')' after $s register\n");
        }
    // TODO: there is no check for garbage after i
    }else if(isin(in.type, 2, 15,16)){ // beq, bne
        in.s = reg_parse(strtok(0, " \t\n"), &in.type, 0, line, err);
        if(in.type == -1) return in;
        in.t = reg_parse(strtok(0, " ,\t\n"), &in.type, 0, line, err);
        if(in.type == -1) return in;

        // parse label or 16-bit signed int
        char *next = strtok(0, " \t\n");
        if(!next){
            in.type = -1;
            SERR("reached end of line; expected 16-bit signed int or label\n");
            return in;
        }else if(isdigit(next[0]) || next[0] == '-'){ // try to parse a number
            in.i = sint_parse(next, 16, &in.type, 0, line, err);
            if(in.type == -1) return in;
        }else{ // try to parse a label
            if(!valid_label(next)){
                in.type = -1;
                WERR("invalid label '%s' referenced\n", next);
                return in;
            }else{
                in.lbl = strdup(next);
            }
        }
    }

    char *c = strtok(0, " \t\n");
    if(c){
        in.type = -1;
        WERR("unexpected token '%s' after valid line\n", c);
    }

    return in;
}

void lbl_replace(struct AVLTree *lbls, struct inst *in, unsigned int line,
                 long int addr, char *err){
    if(in->lbl){
        long int *dat = avl_lookup(lbls, in->lbl);
        if(!dat){
            in->type = -1;
            WERR("referenced label '%s' was never defined\n", in->lbl);
        }else{
            // TODO: are beq, bne, and .word all?
            if(isin(in->type, 2, 15,16)){ // beq, bne
                long int branch = *dat - addr - 1;
                if(branch < -pwr2(15) || branch >= pwr2(15)){
                    in->type = -1;
                    WERR("branch to label '%s' out of range (%ld)\n",
                         in->lbl, branch);
                    return;
                }
                in->i = branch;
            }else if(in->type == 1){ // .word, yo
                if(*dat >= pwr2(31)){
                    in->type = -1;
                    WERR(".word of label '%s' out of range (%ld)\n",
                         in->lbl, *dat);
                    return;
                }
                in->i = *dat * 4;
            }
            free(in->lbl);
            in->lbl = 0;
        }
    }
}

// get the opcode of an instruction type
word get_opcode(type_t t){
    if(t == 1) return 0;
    else if(t == 2) return 32;
    else if(t == 3) return 34;
    else if(t == 4) return 24;
    else if(t == 5) return 25;
    else if(t == 6) return 26;
    else if(t == 7) return 27;
    else if(t == 8) return 16;
    else if(t == 9) return 18;
    else if(t == 10) return 20;
    else if(t == 11) return 35; // on left
    else if(t == 12) return 43; // on left
    else if(t == 13) return 42;
    else if(t == 14) return 43;
    else if(t == 15) return 4; // on left
    else if(t == 16) return 5; // on left
    else if(t == 17) return 8;
    else if(t == 18) return 9;
    else return 999999; // wat
}

// inverse of get_opcode; gets the instruction type of an opcode
type_t get_itype(word w){
    if(w == 0) return 1;
    else if(w == 32) return 2;
    else if(w == 34) return 3;
    else if(w == 24) return 4;
    else if(w == 25) return 5;
    else if(w == 26) return 6;
    else if(w == 27) return 7;
    else if(w == 16) return 8;
    else if(w == 18) return 9;
    else if(w == 20) return 10;
    else if(w == 35) return 11; // on left
    else if(w == 43) return 12; // on left
    else if(w == 42) return 13;
    else if(w == 43) return 14;
    else if(w == 4) return 15; // on left
    else if(w == 5) return 16; // on left
    else if(w == 8) return 17;
    else if(w == 9) return 18;
    else return -1;
}

// convert a signed two's complement n-bit integer to an unsigned n-bit integer
// with the same bits
word toun(int32_t i, int n){
    return i<0 ? i + pwr2(n) : i;
}

// inverse of the above; convert an unsigned int to a bit-equivalent signed int
int32_t tosign(word w, int n){
    return w >= pwr2(n-1) ? w - pwr2(n) : w;
}

word inst_encode(struct inst in){
    word opc = get_opcode(in.type);
    if(in.type == 1){ // .word
        return in.i;
    }else if(isin(in.type, 4, 2,3,13,14)){ // add, sub, slt, sltu
        return opc + (in.d << 11) + (in.t << 16) + (in.s << 21);
    }else if(isin(in.type, 4, 4,5,6,7)){ // mult, multu, div, divu
        return opc + (in.t << 16) + (in.s << 21);
    }else if(isin(in.type, 3, 8,9,10)){ // mfhi, mflo, lis
        return opc + (in.d << 11);
    }else if(isin(in.type, 4, 11,12,15,16)){ // lw, sw, beq, bne
        return toun(in.i, 16) + (in.t << 16) + (in.s << 21) + (opc << 26);
    }else if(isin(in.type, 2, 17,18)){ // jr,jalr
        return opc + (in.s << 21);
    }
    return 0; // unreachable
}

struct inst inst_decode(word w){
    struct inst in;
    in.lbl = 0;

    // parse a bunch of regions of the word
    word leftop = w >> 26;
    word rightop = (w << 26) >> 26;
    type_t ltype = get_itype(leftop);
    type_t rtype = get_itype(rightop);
    in.s = (w << 6) >> 27;
    in.t = (w << 11) >> 27;
    in.d = (w << 16) >> 27;
    in.i = tosign((w << 16) >> 16, 16);
    if(isin(ltype, 4, 11,12,15,16)){ // lw, sw, beq, bne
        in.type = ltype;
    }else if(isin(rtype, 13, 2,3,4,5,6,7,8,9,10,13,14,17,18)){ // all others
        in.type = rtype;
    }else{ // not valid command
        in.type = -1;
    }

    return in;
}
