#include "fb_sys.h"
#include "blitter_top.h"

void fb_sys::init(fb_abs_master & mas)
{
	this->mas = &mas;
}

void fb_sys::fb_set_cyc(fb_cyc_action cyc)
{
	if (cyc == start) {
		cyc_pend = true;
	} else {
		if (state == act_rd) {
			state = idle;
		} else if (state == act_wr) {
			state = wait_sys_tick;
		} else if (state == act_wr_gotdata) {
			state = wait_sys_tick;
		} else if (state == wait_mas_rel) {
			state = idle;
		}

		cyc_pend = false;
		d_wr_pend = false;
	}
}

void fb_sys::fb_set_A(uint32_t addr, bool we)
{
	this->addr = addr;
	this->we = we;
}

void fb_sys::fb_set_D_wr(uint8_t dat)
{
	if (!mas)
		return;

	d_wr = dat;
	d_wr_pend = true;

	if (state == act_wr)
	{
		if ((addr & 0x00FFFFFF) == 0xFFFE30)
		{
			top.set_ROMPG(d_wr);
			top.setDATA(dat | 0x08);			//NOTE: different to hardware! 
		}
		else {
			top.setDATA(dat);
		}
		state = act_wr_gotdata;
		mas->fb_set_ACK(ack);
	}
}

uint8_t fb_sys::peek(uint32_t addr) {
    return (top.sys_readmem)(addr & 0x0000FFFF); // need to restrict to 64k or read/writemem will ignore
}

void fb_sys::poke(uint32_t addr, uint8_t dat) {
    (*top.sys_writemem)(addr & 0x0000FFFF, dat); // need to restrict to 64k or read/writemem will ignore
}

void fb_sys::tick(bool sys)
{
	if (!mas)
		return;
	if (sys) {
		if (state == act_rd) {
			if ((addr & 0xFFFFFF) == 0xFFFE30) {
				mas->fb_set_D_rd(top.get_ROMPG());
			}
			else {
				mas->fb_set_D_rd(top.getDATA());
			}
			mas->fb_set_ACK(ack);
			state = idle;
		}
		else if (state == wait_sys_tick)
		{
			state = idle;
		}
	}

	if (state == idle && cyc_pend) {
		top.ADDR = addr;
		top.RNW = !we;
		if (we) {
			if (d_wr_pend) {
				if ((addr & 0x00FFFFFF) == 0xFFFE30)
				{
					top.set_ROMPG(d_wr);
					top.setDATA(d_wr | 0x08);			//NOTE: different to hardware! 
				}
				else {
					top.setDATA(d_wr);
				}
				state = act_wr_gotdata;
				mas->fb_set_ACK(ack);
			} else {
				state = act_wr;
			}
		}
		else {
			state = act_rd;
		}
	}

}

void fb_sys::reset()
{
	state = idle;
	we = false;
	addr = 0;
	cyc_pend = 0;
	d_wr_pend = 0;
}
