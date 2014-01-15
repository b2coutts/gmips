#include "parse.h"
#include "machine.h"
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

// TODO: read from arg, enforce some upper bound
#define N_BYTES 1000000

typedef unsigned char byte;

int main(int argc, char *argv[]){
    if(argc <= 1){
        fprintf(stderr, "Usage: %s [mips_binary]\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if(!fp){
        fprintf(stderr, "Could not open file \"%s\".\n", argv[1]);
        return 2;
    }

    struct machine m = machine_create(N_BYTES);

    // read code into memory; TODO: binary stream functions?
    int b;
    for(int i = 0; (b = fgetc(fp)) != EOF; i++){
        m.mem[i] = b;
    }

    char *err = malloc(100);
    fprintf(stderr, "Machine output:\n");

    b = machine_adv(&m, err);
    while( b != 1 && b != 9 ){
        if(b == 2) putchar(err[0]);
        b = machine_adv(&m, err);
    }

    fflush(stdout);
    if(b == 1) fprintf(stderr, "\n%s", err);
    fprintf(stderr, "\n");
    machine_dump(m);
}
