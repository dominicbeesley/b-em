/* -*- mode: c; c-basic-offset: 4 -*- */

#ifndef __INCLUDE_B_EM_6502DEBUG__
#define __INCLUDE_B_EM_6502DEBUG__

#include "cpu_debug.h"
#include "debugger.h"

typedef enum {
    M6502,
    M65C02,
    W65816
} m6502_t;


enum register_numbers {
    REG_A,
    REG_X,
    REG_Y,
    REG_S,
    REG_P,
    REG_PC,
    REG_DP,
    REG_DB,
    REG_PB
};

enum {
    F_N = 0x80,
    F_V = 0x40,
    F_E = 0x20, // 65ce02
    F_T = 0x20, // M740: replaces A with $00,X in some opcodes when set
    F_B = 0x10,
    F_D = 0x08,
    F_I = 0x04,
    F_Z = 0x02,
    F_C = 0x01
};


extern const char *dbg6502_reg_names[];
extern size_t dbg6502_print_flags(uint8_t flags, char *buf, size_t bufsize);
extern uint32_t dbg6502_disassemble(cpu_debug_t *cpu, uint32_t addr, char *buf, size_t bufsize, m6502_t model);

#endif
