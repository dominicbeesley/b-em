#include "fbsla_memctl.h"

#include "blitter_top.h"

/* TODO: LOTS nearly all of memctl! */

fbsla_memctl::fbsla_memctl(blitter_top& _top)
	: top(_top)
{
}

fbsla_memctl::~fbsla_memctl()
{
}


void fbsla_memctl::reset()
{
	state = idle;
}

void fbsla_memctl::init(fb_abs_master & mas)
{
	this->mas = &mas;
	reset();
}

void fbsla_memctl::fb_set_cyc(fb_cyc_action cyc)
{
	if (cyc == stop) {
		state = idle;
	}
	else {
		if (we)
			state = actwaitwr;
		else {
			switch (addr & 0x0F)
			{
			case 7:
				mas->fb_set_D_rd(top.get_blturbo());
				break;
			}
			mas->fb_set_ACK(ack);
			state = idle;
		}
	}

}

void fbsla_memctl::fb_set_A(uint32_t addr, bool we)
{
	this->addr = addr;
	this->we = we;
}

void fbsla_memctl::fb_set_D_wr(uint8_t dat)
{
	if (state == actwaitwr)
	{
		switch (addr & 0x0F)
		{
		case 7:
			top.set_blturbo(dat);
			break;
		}
		mas->fb_set_ACK(ack);
		state = idle;
	}
}
