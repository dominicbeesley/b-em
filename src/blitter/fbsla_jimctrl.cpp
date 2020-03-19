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
			if ((addr & 0x0F) == 0x0E)
				mas->fb_set_D_rd(top.get_JIMPAGE_L());
			else if ((addr & 0x0F) == 0x0D)
				mas->fb_set_D_rd(top.get_JIMPAGE_H());
			mas->fb_set_ACK(ack);
		}
	}

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
