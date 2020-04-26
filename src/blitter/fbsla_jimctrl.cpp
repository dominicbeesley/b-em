#include "fbsla_jimctrl.h"
#include "blitter_top.h"


void fbsla_jimctrl::init(fb_abs_master & mas)
{
	this->mas = &mas;
	reset();
}

void fbsla_jimctrl::fb_set_cyc(fb_cyc_action cyc)
{
	if (cyc == stop) {
		state = idle;
	}
	else {
		if (we)
			state = actwaitwr;
		else if (mas)
		{
            mas->fb_set_D_rd(peek(addr));
			mas->fb_set_ACK(ack);
		}
	}

}

uint8_t fbsla_jimctrl::peek(uint32_t addr) {
    if ((addr & 0x0F) == 0x0E)
        return top.get_JIMPAGE_L();
    else if ((addr & 0x0F) == 0x0D)
        return top.get_JIMPAGE_H();
    else
        return 0xFF;
}

void fbsla_jimctrl::poke(uint32_t addr, uint8_t dat) {
    if ((addr & 0x0F) == 0x0E)
        top.set_JIMPAGE_L(dat);
    else if ((addr & 0x0F) == 0x0D)
        top.set_JIMPAGE_H(dat);
}


void fbsla_jimctrl::fb_set_A(uint32_t addr, bool we)
{
	this->addr = addr;
	this->we = we;
}

void fbsla_jimctrl::fb_set_D_wr(uint8_t dat)
{
	if (state == actwaitwr) {
		if ((addr & 0x0F) == 0x0E)
			top.set_JIMPAGE_L(dat);
		else if ((addr & 0x0F) == 0x0D)
			top.set_JIMPAGE_H(dat);
		mas->fb_set_ACK(ack);
		state = idle;
	}
}


void fbsla_jimctrl::reset()
{
	state = idle;
}
