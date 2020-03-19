#include "fbsla_mem_flash.h"
#include <string.h>

void fbsla_mem_flash::init(fb_abs_master & mas)
{
	this->mas = &mas;
	reset();
}

void fbsla_mem_flash::fb_set_cyc(fb_cyc_action cyc)
{
	if (cyc == stop) {
		state = idle;
	}
	else {
		if (we)
			state = actwaitwr;
		else
		{
			if (prog_state == fp_softid)
				if (addr & 1)
					mas->fb_set_D_rd(dev_id);
				else
					mas->fb_set_D_rd(manu_id);
			else {
				if (prog_state & fp_prog)
				{
					prog_toggle = !prog_toggle;
					mas->fb_set_D_rd(
						((~(mem[addr & 0x7FFFF])) & 0x80) //inv top bit
					|   ((prog_toggle)?0x40:0)
					);
				}
				else
				{
					//prog_state = fp_normal;

					mas->fb_set_D_rd(mem[addr & 0x7FFFF]);
				}
			}
			mas->fb_set_ACK(ack);
			state = idle;
		}
	}
}

void fbsla_mem_flash::fb_set_A(uint32_t addr, bool we)
{
	this->addr = addr;
	this->we = we;
}

void fbsla_mem_flash::fb_set_D_wr(uint8_t dat)
{
	if (state == actwaitwr)
	{

		switch (prog_state)
		{
		case fp_normal:
			if (((addr & 0x7FFF) == 0x5555) && dat == 0xAA)
				prog_state = fp_p1;
			break;
		case fp_p1:
			if (((addr & 0x7FFF) == 0x2AAA) && dat == 0x55)
				prog_state = fp_p2;
			else
				prog_state = fp_normal;
			break;
		case fp_p2:
			if (((addr & 0x7FFF) == 0x5555))
			{
				prog_op = dat & 0xF0;
				switch (prog_op)
				{
				case 0xA0:
				case 0x80:
					prog_state = fp_p3;
					break;
				case 0x90:
					prog_state = fp_softid;
					break;
				default:
					prog_state = fp_normal;
				}
			}
			else
				prog_state = fp_normal;
			break;
		case fp_p3:
			if (prog_op == 0xA0) {
				mem[addr & 0x7FFFF] &= dat;		//TODO: check how non-erased fails on real chip
				prog_ct_dn = 20 * 8;
				prog_state = fp_prog;
			}
			else
				if (((addr & 0x7FFF) == 0x5555) && dat == 0xAA)
					prog_state = fp_p4;
				else
					prog_state = fp_normal;
			break;
		case fp_p4:
			if (((addr & 0x7FFF) == 0x2AAA) && dat == 0x55)
				prog_state = fp_p5;
			else
				prog_state = fp_normal;
			break;
		case fp_p5:
			if (((addr & 0x7FFF) == 0x5555) && dat == 0x10)
			{
				//chip erase
				prog_state = fp_prog;
				prog_ct_dn = 100000 * 8;
				memset(mem, 0xFF, 0x80000);
			}
			else if (dat == 0x30)
			{
				//sector erase
				prog_state = fp_prog;
				prog_ct_dn = 25000 * 8;
				memset(mem + (addr & 0x7F000), 0xFF, 0x1000);
			}
			else
				prog_state = fp_normal;
			break;
		default:
			prog_state = fp_normal;
		}




		mas->fb_set_ACK(ack);
		state = idle;
	}
}


void fbsla_mem_flash::reset()
{
	state = idle;
}

void fbsla_mem_flash::tick(bool sys)
{
	if (prog_state & fp_prog) {
		prog_ct_dn--;
		if (prog_ct_dn <= 0)
			prog_state = fp_normal;
	}
}
