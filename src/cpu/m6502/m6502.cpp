// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6502.c

    MOS Technology 6502, original NMOS variant

***************************************************************************/

#include "m65x_device.h"
#include "m6502.h"
#include <iomanip>



void m6502_device::reset()
{

	nmi_state = false;
	irq_state = false;
	irq_taken = false;
	v_state = false;
	inhibit_interrupts = false;
	skip_ints_next = false;
	
	m65x_device::reset();
	NextFn = (m65x_device::StatFn)&m6502_device_reset_0;

}



void m6502_device::init()
{
	m65x_device::init();

	PC = 0x0000;
	A = 0x00;
	X = 0x80;
	Y = 0x00;
	P = 0x36;
	SP = 0x0100;
	TMP = 0x0000;
	TMP2 = 0x00;
	IR = 0x00;
	reset();
}


bool m6502_device::tick() {
	//TODO: halt needs to _not_ halt on writes for NMOS
	if (!halt_state)
	{
		(*(void(*)(m65x_device &cpu))NextFn)(*this);
		return true;
	}
	else {
		return false;
	}
}

std::ostream& operator<<(std::ostream& o, m6502_device &dev)
{
	std::ios_base::fmtflags oldflags = o.flags();
	char oldfill = o.fill();

	o << std::hex;
	o.fill('0');

	o << "A:" << std::setw(2) << (int)dev.A;
	o << ", X:" << std::setw(2) << (int)dev.X;
	o << ", Y:" << std::setw(2) << (int)dev.Y;
	o << ", S:" << std::setw(2) << (int)dev.SP;
	o.width(4);
	o << ",PC:" << std::setw(4) << (int)dev.PC;
	o << ",ADR:" << std::setw(4) << (int)dev.ADDR;
	o << ",RnW:" << (dev.RNW) ? "R" : "W";
	o << ",D:" << std::setw(2) << (int)dev.DAT;
	o << ",IR:" << std::setw(2) << (int)dev.IR;

	o << ", P:";

	int f = dev.P;
	const char flagsTempl[] = "nvobdizc";
	const char *p = flagsTempl;
	for (int i = 0; i < 8; i++) {
		if (f & 0x80)
			o << (char)toupper(*p);
		else
			o << *p;
		*p++;
		f = f << 1;
	}

	//o << ",st:" << std::setw(2) << (int)dev.inst_state << ":" << std::setw(2) << dev.inst_substate;

	if (dev.get_sync()) {
		o << " SYNC";
	}

	o << std::endl;

	o.flags(oldflags);
	o.fill(oldfill);

	return o;
}

void m6502_device::do_adc_d(uint8_t val)
{
	uint8_t c = P & F_C ? 1 : 0;
	P &= ~(F_N | F_V | F_Z | F_C);
	uint8_t al = (A & 15) + (val & 15) + c;
	if (al > 9)
		al += 6;
	uint8_t ah = (A >> 4) + (val >> 4) + (al > 15);
	if (!uint8_t(A + val + c))
		P |= F_Z;
	else if (ah & 8)
		P |= F_N;
	if (~(A^val) & (A ^ (ah << 4)) & 0x80)
		P |= F_V;
	if (ah > 9)
		ah += 6;
	if (ah > 15)
		P |= F_C;
	A = (ah << 4) | (al & 15);
}

void m6502_device::do_adc_nd(uint8_t val)
{
	uint16_t sum;
	sum = A + val + (P & F_C ? 1 : 0);
	P &= ~(F_N | F_V | F_Z | F_C);
	if (!uint8_t(sum))
		P |= F_Z;
	else if (int8_t(sum) < 0)
		P |= F_N;
	if (~(A^val) & (A^sum) & 0x80)
		P |= F_V;
	if (sum & 0xff00)
		P |= F_C;
	A = sum;
}

void m6502_device::do_adc(uint8_t val)
{
	if (P & F_D)
		do_adc_d(val);
	else
		do_adc_nd(val);
}

void m6502_device::do_arr_nd()
{
	bool c = P & F_C;
	P &= ~(F_N | F_Z | F_C | F_V);
	A >>= 1;
	if (c)
		A |= 0x80;
	if (!A)
		P |= F_Z;
	else if (int8_t(A) < 0)
		P |= F_N;
	if (A & 0x40)
		P |= F_V | F_C;
	if (A & 0x20)
		P ^= F_V;
}

void m6502_device::do_arr_d()
{
	// The adc/ror interaction gives an extremely weird result
	bool c = P & F_C;
	P &= ~(F_N | F_Z | F_C | F_V);
	uint8_t a = A >> 1;
	if (c)
		a |= 0x80;
	if (!a)
		P |= F_Z;
	else if (int8_t(a) < 0)
		P |= F_N;
	if ((a ^ A) & 0x40)
		P |= F_V;

	if ((A & 0x0f) >= 0x05)
		a = ((a + 6) & 0x0f) | (a & 0xf0);

	if ((A & 0xf0) >= 0x50) {
		a += 0x60;
		P |= F_C;
	}
	A = a;
}

void m6502_device::do_arr()
{
	if (P & F_D)
		do_arr_d();
	else
		do_arr_nd();
}

void m6502_device::do_cmp(uint8_t val1, uint8_t val2)
{
	P &= ~(F_N | F_Z | F_C);
	uint16_t r = val1 - val2;
	if (!r)
		P |= F_Z;
	else if (int8_t(r) < 0)
		P |= F_N;
	if (!(r & 0xff00))
		P |= F_C;
}

void m6502_device::do_sbc_d(uint8_t val)
{
	uint8_t c = P & F_C ? 0 : 1;
	P &= ~(F_N | F_V | F_Z | F_C);
	uint16_t diff = A - val - c;
	uint8_t al = (A & 15) - (val & 15) - c;
	if (int8_t(al) < 0)
		al -= 6;
	uint8_t ah = (A >> 4) - (val >> 4) - (int8_t(al) < 0);
	if (!uint8_t(diff))
		P |= F_Z;
	else if (diff & 0x80)
		P |= F_N;
	if ((A^val) & (A^diff) & 0x80)
		P |= F_V;
	if (!(diff & 0xff00))
		P |= F_C;
	if (int8_t(ah) < 0)
		ah -= 6;
	A = (ah << 4) | (al & 15);
}

void m6502_device::do_sbc_nd(uint8_t val)
{
	uint16_t diff = A - val - (P & F_C ? 0 : 1);
	P &= ~(F_N | F_V | F_Z | F_C);
	if (!uint8_t(diff))
		P |= F_Z;
	else if (int8_t(diff) < 0)
		P |= F_N;
	if ((A^val) & (A^diff) & 0x80)
		P |= F_V;
	if (!(diff & 0xff00))
		P |= F_C;
	A = diff;
}

void m6502_device::do_sbc(uint8_t val)
{
	if (P & F_D)
		do_sbc_d(val);
	else
		do_sbc_nd(val);
}

void m6502_device::do_bit(uint8_t val)
{
	P &= ~(F_N | F_Z | F_V);
	uint8_t r = A & val;
	if (!r)
		P |= F_Z;
	if (val & 0x80)
		P |= F_N;
	if (val & 0x40)
		P |= F_V;
}

uint8_t m6502_device::do_asl(uint8_t v)
{
	P &= ~(F_N | F_Z | F_C);
	uint8_t r = v << 1;
	if (!r)
		P |= F_Z;
	else if (int8_t(r) < 0)
		P |= F_N;
	if (v & 0x80)
		P |= F_C;
	return r;
}

uint8_t m6502_device::do_lsr(uint8_t v)
{
	P &= ~(F_N | F_Z | F_C);
	if (v & 1)
		P |= F_C;
	v >>= 1;
	if (!v)
		P |= F_Z;
	return v;
}

uint8_t m6502_device::do_ror(uint8_t v)
{
	bool c = P & F_C;
	P &= ~(F_N | F_Z | F_C);
	if (v & 1)
		P |= F_C;
	v >>= 1;
	if (c)
		v |= 0x80;
	if (!v)
		P |= F_Z;
	else if (int8_t(v) < 0)
		P |= F_N;
	return v;
}

uint8_t m6502_device::do_rol(uint8_t v)
{
	bool c = P & F_C;
	P &= ~(F_N | F_Z | F_C);
	if (v & 0x80)
		P |= F_C;
	v <<= 1;
	if (c)
		v |= 0x01;
	if (!v)
		P |= F_Z;
	else if (int8_t(v) < 0)
		P |= F_N;
	return v;
}

uint8_t m6502_device::do_asr(uint8_t v)
{
	P &= ~(F_N | F_Z | F_C);
	if (v & 1)
		P |= F_C;
	v >>= 1;
	if (!v)
		P |= F_Z;
	else if (v & 0x40) {
		P |= F_N;
		v |= 0x80;
	}
	return v;
}


void m6502_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum) {
	case IRQ_LINE: irq_state = state == ASSERT_LINE; break;
	case NMI_LINE: nmi_state = nmi_state || (state == ASSERT_LINE); break;
	case V_LINE:
		if (!v_state && state == ASSERT_LINE)
			P |= F_V;
		v_state = state == ASSERT_LINE;
		break;
	case HALT_LINE:
		halt_state = state == ASSERT_LINE;
		break;
	}
}

void m6502_device::set_nz(uint8_t v)
{
	P &= ~(F_Z | F_N);
	if (v & 0x80)
		P |= F_N;
	if (!v)
		P |= F_Z;
}



/* shared state functions */

void m6502_device_postfetch(m6502_device &cpu)
{
	cpu.IR = cpu.DAT;
	cpu.sync = false;
	//sync_w(CLEAR_LINE);

	if (!cpu.skip_ints_next && (cpu.nmi_state || (cpu.irq_state && !(cpu.P & cpu.F_I))) && !cpu.inhibit_interrupts) {
		cpu.irq_taken = true;
		cpu.IR = 0x00;
	}
	else
		cpu.PC++;
	cpu.skip_ints_next = false;

	cpu.postfetch_int();
}

void m6502_device_prefetch(m6502_device &cpu)
{
	cpu.sync = true;
	//sync_w(ASSERT_LINE);
	cpu.ADDR = cpu.PC;
	cpu.RNW = true;
	cpu.NextFn = (m65x_device::StatFn)cpu.PrefetchNextFn;
}

void m6502_device_fetch(m6502_device &cpu) {
	cpu.PrefetchNextFn = (m65x_device::StatFn)&m6502_device_postfetch;
	m6502_device_prefetch(cpu);
}

void m6502_device_fetch_noirq(m6502_device &cpu) {
	cpu.skip_ints_next = true;
	cpu.PrefetchNextFn = (m65x_device::StatFn)&m6502_device_postfetch;
	m6502_device_prefetch(cpu);
}








