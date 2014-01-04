#include "parse.h"
#include <stdlib.h>

#define RETURN_ADDR 0xffff0010 // TODO: unhardcode?

typedef unsigned char byte;

struct machine {
    byte *mem;  // memory for the virtual machine
    size_t n;   // size of above array

    word *reg;  // array of size 31 containing registers 1-31
    word pc;    // program counter

    // hi and lo destination registers for mult and multu
    word hi;
    word lo;
};

// initialize a machine with n bytes of memory
struct machine machine_create(size_t n);

// deallocate all of the memory allocated by a machine
void machine_destroy(struct machine m);

// advance the machine by 1 step; on success return 0, on termination (i.e., jr
// $31) return 9, on failure return 1 and write an error to err
int machine_adv(struct machine *m, char *err);

// dump the contents of the registers and PC to stdout
void machine_dump(struct machine m);
