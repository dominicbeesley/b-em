#include "fb_slave_default.h"

void fb_def_slave::init(fb_abs_master & mas)
{
	this->mas = &mas;
}

void fb_def_slave::fb_set_cyc(fb_cyc_action cyc)
{
	if (!mas)
		return;
	if (cyc == start)
		mas->fb_set_ACK(ack);

}

void fb_def_slave::fb_set_A(uint32_t addr, bool we)
{
	//ignored
}

void fb_def_slave::fb_set_D_wr(uint8_t dat)
{
	//ignored
}
