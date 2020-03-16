// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6502.h

    MOS Technology 6502, original NMOS variant

***************************************************************************/

#ifndef MAME_CPU_M6502_M6502_H
#define MAME_CPU_M6502_M6502_H


#include "m65x_device.h"
#include <iostream>

class m6502_device;
#include "m6502_top.gen.h"

/* shared state functions */
void m6502_device_postfetch(m6502_device &cpu);
void m6502_device_prefetch(m6502_device &cpu);
void m6502_device_fetch(m6502_device &cpu);
void m6502_device_fetch_noirq(m6502_device &cpu);


class m6502_device : public m65x_device {
public:


	m6502_device(void) :
		m65x_device()
		, PC(0), SP(0), TMP(0), TMP2(0), A(0), X(0), Y(0), P(0), IR(0),
		NextFn(0), nmi_state(false), irq_state(false), v_state(false), irq_taken(false),
		inhibit_interrupts(false)
	{
	};


	virtual void init() override;

	bool tick() override;

	friend std::ostream& operator<<(std::ostream& o, m6502_device &dev);

	virtual void execute_set_input(int inputnum, int state) override;

	virtual void reset() override;

	uint16_t getPC() { return PC; };
	uint16_t getSP() { return SP; };
	uint8_t getA() { return A; };
	uint8_t getX() { return X; };
	uint8_t getY() { return Y; };
	uint8_t getP() { return P; };

	void setPC(uint16_t val) { PC = val; };
	void setSP(uint16_t val) { SP = val; };
	void setA(uint8_t val) { A = val; };
	void setX(uint8_t val) { X = val; };
	void setY(uint8_t val) { Y = val; };
	void setP(uint8_t val) { P = val; };


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


protected:

	StatFn	  NextFn;
	StatFn	  PrefetchNextFn;


	friend void m6502_device_postfetch(m6502_device &cpu);
	friend void m6502_device_prefetch(m6502_device &cpu);
	friend void m6502_device_fetch(m6502_device &cpu);
	friend void m6502_device_fetch_noirq(m6502_device &cpu);


	uint16_t  PC;                     /* program counter */
	uint16_t  SP;                     /* stack pointer (always 100 - 1FF) */
	uint16_t  TMP;                    /* temporary internal values */
	uint8_t   TMP2;                   /* another temporary internal value, 8 bits this time */
	uint8_t   A;                      /* Accumulator */
	uint8_t   X;                      /* X index register */
	uint8_t   Y;                      /* Y index register */
	uint8_t   P;                      /* Processor status */
	uint8_t   IR;                     /* Prefetched instruction register */


	bool	  skip_ints_next;			  /* Do not check for interrupts on this fetch*/


	bool nmi_state, irq_state, v_state, halt_state;
	bool irq_taken, inhibit_interrupts;

	void set_nz(uint8_t v);


	// inline helpers
	bool page_changing(uint16_t base, int delta) { return (((base + delta) ^ base) & 0xff00) != 0; }
	uint16_t set_l(uint16_t base, uint8_t val) { return (base & 0xff00) | val; }
	uint16_t set_h(uint16_t base, uint8_t val) { return (base & 0x00ff) | (val << 8); }

	void dec_SP() { SP = set_l(SP, SP - 1); }
	void inc_SP() { SP = set_l(SP, SP + 1); }

	void do_adc_d(uint8_t val);
	void do_adc_nd(uint8_t val);
	void do_sbc_d(uint8_t val);
	void do_sbc_nd(uint8_t val);
	void do_arr_d();
	void do_arr_nd();

	void do_adc(uint8_t val);
	void do_cmp(uint8_t val1, uint8_t val2);
	void do_sbc(uint8_t val);
	void do_bit(uint8_t val);
	void do_arr();
	uint8_t do_asl(uint8_t v);
	uint8_t do_lsr(uint8_t v);
	uint8_t do_ror(uint8_t v);
	uint8_t do_rol(uint8_t v);
	uint8_t do_asr(uint8_t v);


#include "m6502.gen.h"



};




#endif // MAME_CPU_M6502_M6502_H
