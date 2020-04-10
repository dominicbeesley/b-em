#ifndef __INC_SYS_H
#define __INC_SYS_H

/*
Dominic Beesley 2020 - separate out main system board functions from 6502.c
*/

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "6502debug.h"

void sys_reset(void);
void sys_exec(void);

extern FILE *hogrec_fp;
void hogrec_start(const char *filename);

extern int interrupt;
extern int nmi;

void os_paste_start(char *str);

uint8_t readmem(uint16_t addr);
void writemem(uint16_t addr, uint8_t val);

extern cpu_debug_t core6502_cpu_debug;

//TODO: rename
void m6502_savestate(FILE *f);
void m6502_loadstate(FILE *f);

//TODO: rename, should dump main CPU regs
void dumpregs(void);
//TODO: should return main cpu pc at start of instruction
extern uint16_t cpu_cur_op_pc;

void sys_sound_fillbuf(int16_t *buffer, int len);

long get_blitter_ticks();

#endif
