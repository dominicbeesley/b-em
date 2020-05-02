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
    top.set_blturbo(0);
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
            mas->fb_set_D_rd(peek(addr));
			mas->fb_set_ACK(ack);
			state = idle;
		}
	}

}

uint8_t fbsla_memctl::peek(uint32_t addr) {
    switch (addr & 0x0F)
    {
    case 7:
        return top.get_blturbo();
        break;
    default:
        return 0xFF;
        break;
    }
}

void fbsla_memctl::poke(uint32_t addr, uint8_t dat) {
    switch (addr & 0x0F)
    {
    case 7:
        return top.set_blturbo(dat);
        break;
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
