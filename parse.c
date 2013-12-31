// TODO: replace %i with %d in format strings?
#include "parse.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>

#define ERR "assembler: ERROR: line %u: "
// TODO: replace everything with WERR if it works
#define WERR(msg, ...) sprintf(err, ERR msg, line, __VA_ARGS__);
// #define WERR(msg, ...) sprintf(err, "testing%i", line, __VA_ARGS__);

// minimum and maximum values
#define MIN_REG 0
#define MAX_REG 31
#define MAX_32 2147483647
#define MIN_32 (-2147483648)


// true iff the given string is a valid label
// TODO: find the spec and make this conform to it
int valid_label(const char *str){
    while(str[0] != '\0'){
        if(!isalpha(str[0])) return 0;
        str++;
    }
    return 1;
}

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
        if(!isspace(str[i])) return 0;
    }
    return 1;
}

// attempt to parse a signed n-bit integer from the beginning of str
    // on success: return the int; put rest of string in *r if r != 0
    // on failure: set *t = -1, and write an error message to err
int32_t sint_parse(char *str, int n, type_t *t, char **r, unsigned int line,
                   char *err){
    if(!str){
        *t = -1;
        sprintf(err, ERR "reached end of line; expected %d-bit signed "
                "integer\n", line, n);
        return 0;
    }

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
    if(!str){
        *t = -1;
        sprintf(err, ERR "reached end of line; expected register\n", line);
        return 0;
    }

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

struct inst inst_parse(char *str, unsigned int line, char *err,
                       unsigned int addr, struct AVLTree *lbls){
    struct inst in;
    in.lbl = 0;
    if(isempty(str)){
        in.type = 0;
        return in;
    }

    // parse any labels from the beginning of the line
    char *typestr = strtok(str, " \t\n");
    while(typestr[strlen(typestr)-1] == ':'){
        typestr[strlen(typestr)-1] = '\0';
        if(!valid_label(typestr)){
            in.type = -1;
            sprintf(err, ERR "invalid label '%s' declared\n", line, typestr);
            return in;
        }
        avl_insert(lbls, typestr, addr);
        typestr[strlen(typestr)-1] = ':';
        typestr = strtok(0, " \t\n");
    }
    in.type = gettype(typestr);
    if(in.type == -1){
        sprintf(err, ERR "invalid operation '%s'\n", line, typestr);
        return in;
    }

    // TODO: does not ensure commas are there right now
    // TODO: only allows decimal numbers (no hex)
    if(in.type == 1){ // .word
        in.i = sint_parse(strtok(0, " \t\n"), 32, &in.type, 0, line, err);
        if(in.type == -1) return in;
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
        in.s = reg_parse(strtok(0, " \t\n"), &in.type, 0, line, err);
        if(in.type == -1) return in;
        in.t = reg_parse(strtok(0, " ,\t\n"), &in.type, 0, line, err);
        if(in.type == -1) return in;

        // parse label or 16-bit signed int
        char *next = strtok(0, " \t\n");
        if(!next){
            in.type = -1;
            sprintf(err, ERR "reached end of line; expected 16-bit signed int "
                             "or label\n", line);
            return in;
        }else if(isdigit(next[0])){ // try to parse a number
            in.i = sint_parse(next, 16, &in.type, 0, line, err);
            if(in.type == -1) return in;
        }else{ // try to parse a label
            if(!valid_label(next)){
                //sprintf(err, ERR "invalid label '%s' referenced\n",
                    // line, typestr);
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
        sprintf(err, ERR "unexpected token '%s' after valid line\n", line, c);
    }

    return in;
}
