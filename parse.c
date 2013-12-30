#include "parse.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>

// TODO: why are these not coming from parse.h?
#include <stdint.h>
typedef char type_t;

#define ERR "assembler: ERROR: line %u: "

// TODO: needed?
#define CHK char *tk = strtok(str, " \t"); if(tk){ in.type = -1; \
    sprintf(err, ERR "unexpected token '%s'", line, tk); return in; }
    

// minimum and maximum values
#define MIN_REG 0
#define MAX_REG 31
#define MAX_32 2147483647
#define MIN_32 (-2147483648)

// duplicate a string into a new location in memory
// TODO: remove if not using
char *strdup(const char *str){
    char *ret = malloc(strlen(str)+1);
    return strcpy(ret, str);
}

// true iff str is in one of the optional arguments
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
        if(i == ';') return 1;
        if(!isspace(i)) return 0;
    }
    return 1;
}

// attempt to parse a signed n-bit integer from the beginning of str
    // on success: return the int; put rest of string in *r if r != 0
    // on failure: set *t = -1, and write an error message to err
int32_t sint_parse(char *str, int n, type_t *t, char **r, unsigned int line,
                   char *err){
    char *ret;
    long int i = strtol(str, &ret, 10);
    if(ret == str){
        *t = -1;
        sprintf(err, ERR "failed to parse %i-bit signed int\n", line, n);
        return 0;
    }else if(i < MIN_32 || i > MAX_32){
        *t = -1;
        sprintf(err, ERR "%i-bit signed int '%ld' out of range\n", line, n, i);
        return 0;
    }

    if(r) *r = ret;
    return i;
}

// attempt to parse a register from the beginning of str
    // on success: return the int; put rest of string in *r if r != 0
    // on failure: set *t = -1, and write an error message to err
uint8_t reg_parse(char *str, type_t *t, char **r, unsigned int line, char *err){
    while(isspace(str[0])) str++;
    if(str[0] != '$'){
        *t = -1;
        sprintf(err, ERR "expected '$' before register number\n", line);
        return 0;
    }
    
    char *ret;
    long int i = strtol(str+1, &ret, 10);
    if(ret == str){
        *t = -1;
        sprintf(err, ERR "failed to parse register number\n", line);
        return 0;
    }else if(i < MIN_REG || i > MAX_REG){
        *t = -1;
        sprintf(err, ERR "register number '%ld' out of range\n", line, i);
        return 0;
    }

    if(r) *r = ret;
    return i;
}

// return the typecode for the string
// TODO: ignore case?
type_t gettype(const char *str){
    if(strcmp(str, ".word") == 0) return 1;
    else if(strcmp(str, "add") == 0) return 2;
    else if(strcmp(str, "sub") == 0) return 3;
    else if(strcmp(str, "mult") == 0) return 4;
    else if(strcmp(str, "multu") == 0) return 5;
    else if(strcmp(str, "div") == 0) return 6;
    else if(strcmp(str, "divu") == 0) return 7;
    else if(strcmp(str, "mfi") == 0) return 8;
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


struct inst inst_parse(char *str, unsigned int line, char *err){
    struct inst in;
    if(isempty(str)){
        in.type = 0;
        return in;
    }

    // parse instruction type (i.e. opcode)
    char *typestr = strtok(str, " \t\n");
    in.type = gettype(typestr);
    if(in.type == -1){
        sprintf(err, ERR "invalid operation '%s'\n", line, typestr);
        return in;
    }

    // TODO: does not ensure commas are there right now
    // TODO: only allows decimal numbers (no hex)
    if(in.type == 1){ // .word
        in.i = sint_parse(strtok(0, " \t"), 32, &in.type, 0, line, err);
        if(in.type == -1) return in;
    }else if(isin(in.type, 4, 2,3,13,14)){ // add, sub, slt, sltu
        in.d = reg_parse(strtok(0, " \t"), &in.type, 0, line, err);
        if(in.type == -1) return in;
        in.s = reg_parse(strtok(0, " ,\t"), &in.type, 0, line, err);
        if(in.type == -1) return in;
        in.t = reg_parse(strtok(0, " ,\t"), &in.type, 0, line, err);
        if(in.type == -1) return in;
    }else if(isin(in.type, 4, 4,5,6,7)){ // mult, multu, div, divu
        in.s = reg_parse(strtok(0, " \t"), &in.type, 0, line, err);
        if(in.type == -1) return in;
        in.t = reg_parse(strtok(0, " ,\t"), &in.type, 0, line, err);
        if(in.type == -1) return in;
    }else if(isin(in.type, 3, 8,9,10)){ // mfhi, mflo, lis
        in.d = reg_parse(strtok(0, " \t"), &in.type, 0, line, err);
        if(in.type == -1) return in;
    }else if(isin(in.type, 2, 17,18)){ // jr, jalr
        in.s = reg_parse(strtok(0, " \t"), &in.type, 0, line, err);
        if(in.type == -1) return in;
    }else if(isin(in.type, 2, 11,12)){ // lw, sw
        in.d = reg_parse(strtok(0, " \t"), &in.type, 0, line, err);
        if(in.type == -1) return in;
        char *next = strtok(0, " \t");
        in.i = sint_parse(next, 16, &in.type, &next, line, err);
        if(in.type == -1) return in;
        if(next[0] != '('){
            in.type = -1;
            sprintf(err, ERR "expected '(' before $s register\n", line);
            return in;
        }

        in.s = reg_parse(next+1, &in.type, &next, line, err);
        if(in.type == -1) return in;
        if(next[0] != ')'){
            in.type = -1;
            sprintf(err, ERR "expected ')' after $s register\n", line);
        }
    // TODO: there is no check for garbage after i
    }else if(isin(in.type, 2, 15,16)){ // beq, bne
        in.s = reg_parse(strtok(0, " \t"), &in.type, 0, line, err);
        if(in.type == -1) return in;
        in.t = reg_parse(strtok(0, " ,\t"), &in.type, 0, line, err);
        if(in.type == -1) return in;
        char *next = strtok(0, " \t");
        in.i = sint_parse(next, 16, &in.type, 0, line, err);
        if(in.type == -1) return in;
    }

    char *c = strtok(0, " \t");
    if(c && c[0] != ';'){
        in.type = -1;
        sprintf(err, ERR "unexpected token '%s' after valid line\n", line, c);
    }

    return in;
}