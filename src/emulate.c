#include "parse.h"
#include "machine.h"
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

// TODO: read from arg, enforce some upper bound
#define N_BYTES 1000000

typedef unsigned char byte;

int main(){
    struct machine m = machine_create(N_BYTES);

    // read code into memory; TODO: binary stream functions?
    int b;
    for(int i = 0; (b = getchar()) != -1; i++){
        m.mem[i] = b;
    }

    char *err = malloc(100);
    printf("Machine output:\n");

    b = machine_adv(&m, err);
    while( b != 1 && b != 9 ){
        if(b == 2) putchar(err[0]);
        b = machine_adv(&m, err);
    }
    if(b == 1) fprintf(stderr, "\n%s", err);
    printf("\n");
    machine_dump(m);
}
