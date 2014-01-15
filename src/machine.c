#include "machine.h"
#include <stdlib.h>
#include <stdio.h>

// read/set register macros
#define RR(x) (x ? m->reg[x-1] : 0)
#define SR(x,y) if(x) m->reg[x-1] = y;

// merge 4 bytes (b[0] .. b[3]) into a word
word merge(byte *b){
    return (b[0] << 24) + (b[1] << 16) + (b[2] << 8) + b[3];
}

// convert a signed two's complement n-bit integer to an unsigned n-bit integer
// with the same bits
word mtoun(int32_t i){
    return i<0 ? i + 4294967296 : i;
}

// inverse of the above; convert an unsigned int to a bit-equivalent signed int
int32_t mtosign(word w){
    return w >= 2147483648 ? w - 4294967296 : w;
}

struct machine machine_create(size_t n){
    struct machine m;
    m.mem = calloc(n, 1); // TODO: does the memory need to be zeroed?
    m.n = n;
    m.reg = calloc(31,4);
    m.reg[30] = RETURN_ADDR;
    m.reg[29] = n;
    m.pc = 0;
    m.hi = 0;
    m.lo = 0;
    return m;
}

void machine_destroy(struct machine m){
    free(m.mem);
    free(m.reg);
}

int machine_adv(struct machine *m, char *err){
    if(m->pc + 4 >= m->n){
        sprintf(err, "emulator: ERROR: PC is %08u, but memory ends at %08lu; "
                     "no word to execute\n", m->pc, m->n);
        return 1;
    }
    word w = merge(m->mem + m->pc);
    struct inst in = inst_decode(w);
    if(in.type == -1){
        sprintf(err, "emulator: ERROR: could not decode word 0x%08x as "
                     "instruction at memory location %08u\n", w, m->pc);
        return 1;
    }else if(in.type == 2){ // add
        SR(in.d, RR(in.s) + RR(in.t))
    }else if(in.type == 3){ // sub
        SR(in.d, RR(in.s) - RR(in.t))
    }else if(in.type == 4){ // mult
        int64_t prod = mtosign(RR(in.s)) * mtosign(RR(in.t));
        m->lo = mtoun((prod << 32) >> 32);
        m->hi = mtoun(prod >> 32);
    }else if(in.type == 5){ // multu
        uint64_t prod = RR(in.s) * RR(in.t);
        m->lo = (prod << 32) >> 32;
        m->hi = prod >> 32;
    }else if(in.type == 6){ // div
        m->lo = mtoun(mtosign(RR(in.s)) / mtosign(RR(in.t)));
        m->hi = mtoun(mtosign(RR(in.s)) % mtosign(RR(in.t)));
    }else if(in.type == 7){ // divu
        m->lo = RR(in.s) / RR(in.t);
        m->hi = RR(in.s) % RR(in.t);
    }else if(in.type == 8){ // mfhi
        SR(in.d, m->hi);
    }else if(in.type == 9){ // mflo
        SR(in.d, m->lo);
    }else if(in.type == 10){ // lis
        if(m->pc + 8 >= m->n){
            sprintf(err, "emulator: ERROR: lis instruction at memory location "
                         "%08u, but memory ends at %08lu; no word follows it\n",
                         m->pc, m->n);
            return 1;
        }else{
            SR(in.d, merge(m->mem + m->pc + 4));
            m->pc+=4;
        }
    }else if(in.type == 11){ // lw
        word addr = RR(in.s) + in.i;
        if(addr == INPUT_ADDR){ // read in highest bytes from STDIN
            SR(in.t, (word) getchar());
            m->pc += 4;
            return 0;
        }
        if(addr > m->n-4){ // check for loads beyond memory
            sprintf(err, "emulator: ERROR: attempted lw at address 0x%08x, but"
                         " memory ends at 0x%08x\n", addr, (unsigned int)m->n);
            return 1;
        }
        SR(in.t, m->mem[addr]);
    }else if(in.type == 12){ // sw
        word addr = RR(in.s) + in.i;
        if(addr == OUTPUT_ADDR){ // write lowest byte to STDOUT
            err[0] = (RR(in.t) << 24) >> 24;
            m->pc += 4;
            return 2;
        }
        if(addr > m->n-4){ // check for stores beyond memory
            sprintf(err, "emulator: ERROR: attempted sw at address 0x%08x, but"
                         " memory ends at 0x%08x\n", addr, (unsigned int)m->n);
            return 1;
        }
        m->mem[addr] = RR(in.t);
    }else if(in.type == 13){ // slt
        SR(in.d, mtosign(RR(in.s)) < mtosign(RR(in.t)) ? 1 : 0);
    }else if(in.type == 14){ // sltu
        SR(in.d, RR(in.s) < RR(in.t) ? 1 : 0);
    }else if(in.type == 15){ // beq
        if(RR(in.s) == RR(in.t)) m->pc += in.i * 4;
    }else if(in.type == 16){ // bne
        if(RR(in.s) != RR(in.t)) m->pc += in.i * 4;
    }else if(in.type == 17){ // jr
        m->pc = RR(in.s);
        if(m->pc == RETURN_ADDR) return 9;
    }else if(in.type == 18){ // jalr
        word s = RR(in.s);
        SR(31, m->pc);
        m->pc = s;
    }
    m->pc += 4;
    return 0;
}

void machine_dump(struct machine m){
    fprintf(stderr, "State of MIPS virtual machine:\n");
    fprintf(stderr, " PC = 0x%08x (%010u)          ", m.pc, m.pc);
    fprintf(stderr, "hi = 0x%08x    lo = 0x%08x\n\n", m.hi, m.lo);

    for(int i = 1; i < 32; i+=4){
        for(int j = i; j < i+4 && j < 32; j++){
            fprintf(stderr, "%s$%d = 0x%08x", (j<10 ? " " : ""), j, m.reg[j-1]);
            if(j != i+4) fprintf(stderr, "   ");
        }
        fprintf(stderr, "\n");
    }
}
