// cpu_debug.h

#ifndef CPU_DEBUG_H
#define CPU_DEBUG_H

#include <stdlib.h>
#include <inttypes.h>
#include "debugger_symbols.h"

#define WIDTH_8BITS  0
#define WIDTH_16BITS 1
#define WIDTH_32BITS 2

typedef struct cpu_debug_t {
  const char *cpu_name;                                               // Name/model of CPU.
  int      (*debug_enable)(int newvalue);                             // enable/disable debugging on this CPU, returns previous value.
  uint32_t (*memread)(uint32_t addr);                                 // CPU's usual memory read function.
  void     (*memwrite)(uint32_t addr, uint32_t value);                // CPU's usual memory write function.
  uint32_t (*ioread)(uint32_t addr);                                  // CPU's usual I/O read function.
  void     (*iowrite)(uint32_t addr, uint32_t value);                 // CPU's usual I/O write function.
  uint32_t (*disassemble)(cpu_debug_t *cpu, uint32_t addr, char *buf, size_t bufsize);  // disassemble one line, returns next address
  const char **reg_names;                                             // NULL pointer terminated list of register names.
  uint32_t (*reg_get)(int which);                                     // Get a register - which is the index into the names above
  void     (*reg_set)(int which, uint32_t value);                     // Set a register.
  size_t   (*reg_print)(int which, char *buf, size_t bufsize);        // Print register value in CPU standard form.
  void     (*reg_parse)(int which, const char *strval);               // Parse a value into a register.
  uint32_t (*get_instr_addr)(void);                                   // Returns the base address of the currently executing instruction
  const char **trap_names;                                            // Null terminated list of other reasons a CPU may trap to the debugger.
  const int mem_width;                                                // Width of value returned from memread(): 0=8-bit, 1=16-bit, 2=32-bit
  const int io_width;                                                 // Width of value returned from  ioread(): 0=8-bit, 1=16-bit, 2=32-bit
  const int default_base;                                             // Allows a co pro to override the default base of 16
  size_t   (*print_addr)(cpu_debug_t *cpu, uint32_t addr, char *buf, size_t bufsize, bool include_symbol);   // Print an address.
  symbol_table *symbols;                                              // symbol table for storing symbolic addresses
  uint32_t (*get_program_bank)(void);                                 // 65816 program bank
  uint32_t (*get_data_bank)(void);                                    // 65816 data bank
} cpu_debug_t;

extern void debug_memread (cpu_debug_t *cpu, uint32_t addr, uint32_t value, uint8_t size);
extern void debug_memwrite(cpu_debug_t *cpu, uint32_t addr, uint32_t value, uint8_t size);
extern void debug_ioread  (cpu_debug_t *cpu, uint32_t addr, uint32_t value, uint8_t size);
extern void debug_iowrite (cpu_debug_t *cpu, uint32_t addr, uint32_t value, uint8_t size);
extern void debug_preexec (cpu_debug_t *cpu, uint32_t addr);
extern void debug_trap    (cpu_debug_t *cpu, uint32_t addr, int reason);

extern size_t debug_print_8bit(uint32_t addr, char *buf, size_t bufsize);
extern size_t debug_print_16bit(uint32_t addr, char *buf, size_t bufsize);
extern size_t debug_print_32bit(uint32_t addr, char *buf, size_t bufsize);

extern size_t debug_print_addr16(cpu_debug_t *cpu, uint32_t addr, char *buf, size_t bufsize, bool include_symbol);
extern size_t debug_print_addr32(cpu_debug_t *cpu, uint32_t addr, char *buf, size_t bufsize, bool include_symbol);

#endif
