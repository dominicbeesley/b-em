#include "fbsla_mem_chipram.h"

void fbsla_mem_chipram::init(fb_abs_master & mas)
{
	this->mas = &mas;
	reset();
}

void fbsla_mem_chipram::fb_set_cyc(fb_cyc_action cyc)
{
	if (cyc == stop) {
		state = idle;
	}
	else {
		if (we)
			state = actwaitwr;
		else
		{
			mas->fb_set_D_rd(mem[addr & 0x1FFFFF]);
			mas->fb_set_ACK(ack);
			state = idle;
		}
	}
}

void fbsla_mem_chipram::fb_set_A(uint32_t addr, bool we)
{
	this->addr = addr;
	this->we = we;
}

void fbsla_mem_chipram::fb_set_D_wr(uint8_t dat)
{
	if (state == actwaitwr)
	{
		mem[addr & 0x1FFFFF] = dat;
		mas->fb_set_ACK(ack);
		state = idle;
	}
}


void fbsla_mem_chipram::reset()
{
	state = idle;
}
