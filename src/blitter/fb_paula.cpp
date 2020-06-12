#include "fb_paula.h"

#include <iostream>

fb_paula::~fb_paula()
{
}

void fb_paula::reset()
{
	ChannelSel = 0;
	Volume = 0x3F;
	for (int i = 0; i < NUM_CHANNELS; i++) {
		ChannelRegs[i].data = 0;
		ChannelRegs[i].addr = 0;
		ChannelRegs[i].addr_bank = 0;

		ChannelRegs[i].act = false;
		ChannelRegs[i].act_prev = false;
		ChannelRegs[i].repeat = false;

		ChannelRegs[i].len = 0;
		ChannelRegs[i].period_h_latch = 0;
		ChannelRegs[i].period = 0;
		ChannelRegs[i].vol = 0;
		ChannelRegs[i].peak = 0;

		ChannelRegs[i].repoff = 0;

		ChannelRegs[i].clken_sam_next = false;

	}
	sla.reset();
	mas.reset();

	//clear outbuf
	std::queue<uint16_t> tmp;
	std::swap(outbuf, tmp);
	outbufempty = true; 

}

void fb_paula::tick(bool sys)
{


	//do this once every 3.5MhZ
	paula_clock_acc += PAULA_PCLK_A;
	while (paula_clock_acc > PAULA_PCLK_LIM)
	{
		paula_clock_acc -= PAULA_PCLK_LIM;

		for (int i = 0; i < NUM_CHANNELS; i++)
		{
			paula_CHANNELREGS *curchan = &ChannelRegs[i];

			if (!curchan->act) {
				curchan->samper_ctr = 0;
				curchan->clken_sam_next = 0;
			}
			else {
				if (!curchan->act_prev)
				{
					curchan->clken_sam_next = true;
					curchan->samper_ctr = curchan->period;
				}
				else if (curchan->samper_ctr == 0)
				{
					curchan->clken_sam_next = true;
					curchan->samper_ctr = curchan->period;
					curchan->data = curchan->data_next;
				}
				else
					curchan->samper_ctr--;
			}

			curchan->act_prev = curchan->act;
		}

	}


	//this is a very rough approximation of what really happens in terms of prioritisation
	//but should be close enough - normally data accesses that clash will be queued up
	//by intcon and could take many 8M cycles but here we just always deliver the data!
	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		paula_CHANNELREGS *curchan = &ChannelRegs[i];

		if (curchan->clken_sam_next)
		{
			int addr = (((int)curchan->addr_bank) << 16) + (int)curchan->addr + (int)curchan->sam_ctr;
			if (mas.getByte(addr, i))
				curchan->clken_sam_next = false;

			if (curchan->sam_ctr == curchan->len)
			{
				if (curchan->repeat)
				{
					curchan->sam_ctr = curchan->repoff;
				}
				else
				{
					curchan->act = false;
					curchan->sam_ctr = 0;
				}
			}
			else
			{
				curchan->sam_ctr++;
			}

			break;
		}
	}

	/************************ stream out sound data from channels ***************************/
	sample_clock_acc++;
	while (sample_clock_acc >= PAULA_CYCLES_PER_STREAM_SAMPLE)
	{

		if (outbuf.size() < PAULA_BUFLENMAX) {
			int sample = 0;

			sample = 0;

			for (int i = 0; i < NUM_CHANNELS; i++)
			{
				int snd_dat = (int)ChannelRegs[i].data * (int)(ChannelRegs[i].vol >> 2);
				sample += snd_dat;
				int snd_mag = ((snd_dat > 0) ? snd_dat : -snd_dat) >> 6;
				if (snd_mag > ChannelRegs[i].peak)
					ChannelRegs[i].peak = snd_mag;
			}
			sample = sample * (Volume >> 2) / NUM_CHANNELS;
			sample = sample / 4;
			outbuf.push((uint16_t)sample);
		} else {
			std::cerr << "psnd:wr:full\n";
		}

		sample_clock_acc -= PAULA_CYCLES_PER_STREAM_SAMPLE;
	}



}


void fb_paula::init() {

	reset();
}

void fb_paula::gotByte(uint8_t channel, int8_t dat)
{
	ChannelRegs[channel].data_next = dat;
}

void fb_paula::write_regs(uint8_t addr, uint8_t dat)
{
	// sound registers
			//note registers are all exposed big-endian style
	switch (addr & 0x0F)
	{
	case 0:
		ChannelRegs[ChannelSel].data = dat;
		break;
	case 1:
		ChannelRegs[ChannelSel].addr_bank = dat;
		break;
	case 2:
		ChannelRegs[ChannelSel].addr = (ChannelRegs[ChannelSel].addr & 0x00FF) | (dat << 8);
		break;
	case 3:
		ChannelRegs[ChannelSel].addr = (ChannelRegs[ChannelSel].addr & 0xFF00) | dat;
		break;
	case 4:
		ChannelRegs[ChannelSel].period_h_latch = dat;
		break;
	case 5:
		ChannelRegs[ChannelSel].period = (ChannelRegs[ChannelSel].period_h_latch << 8) | dat;
		break;
	case 6:
		ChannelRegs[ChannelSel].len = (ChannelRegs[ChannelSel].len & 0x00FF) | (dat << 8);
		break;
	case 7:
		ChannelRegs[ChannelSel].len = (ChannelRegs[ChannelSel].len & 0xFF00) | dat;
		break;
	case 8:
		ChannelRegs[ChannelSel].act = (dat & 0x80) != 0;
		ChannelRegs[ChannelSel].repeat = (dat & 0x01) != 0;
		ChannelRegs[ChannelSel].sam_ctr = 0;
		break;
	case 9:
		ChannelRegs[ChannelSel].vol = dat & 0xFC;
		break;
	case 10:
		ChannelRegs[ChannelSel].repoff = (ChannelRegs[ChannelSel].repoff & 0x00FF) | (dat << 8);
		break;
	case 11:
		ChannelRegs[ChannelSel].repoff = (ChannelRegs[ChannelSel].repoff & 0xFF00) | dat;
		break;
	case 12:
		ChannelRegs[ChannelSel].peak = 0;
		break;
	case 14:
		Volume = dat & 0xFC;
		break;
	case 15:
		ChannelSel = dat % NUM_CHANNELS;
		break;
	}

}

uint8_t fb_paula::read_regs(uint8_t addr) {
	//note registers are all exposed big-endian style
	switch (addr & 0x0F)
	{
	case 0:
		return ChannelRegs[ChannelSel].data;
		break;
	case 1:
		return ChannelRegs[ChannelSel].addr_bank;
		break;
	case 2:
		return ChannelRegs[ChannelSel].addr >> 8;
		break;
	case 3:
		return (uint8_t)ChannelRegs[ChannelSel].addr;
		break;
	case 4:
		return ChannelRegs[ChannelSel].period >> 8;
		break;
	case 5:
		return (uint8_t)ChannelRegs[ChannelSel].period;
		break;
	case 6:
		return ChannelRegs[ChannelSel].len >> 8;
		break;
	case 7:
		return (uint8_t)ChannelRegs[ChannelSel].len;
		break;
	case 8:
		return (ChannelRegs[ChannelSel].act ? 0x80 : 0x00) | (ChannelRegs[ChannelSel].repeat ? 0x01 : 0x00);
		break;
	case 9:
		return ChannelRegs[ChannelSel].vol;
		break;
	case 10:
		return ChannelRegs[ChannelSel].repoff >> 8;
		break;
	case 11:
		return (uint8_t)ChannelRegs[ChannelSel].repoff;
		break;
	case 12:
		return ChannelRegs[ChannelSel].peak;
		break;
	case 14:
		return Volume; 
		break;
	case 15:
		return ChannelSel;
		break;
	default:
		return 255;
		break;
	}

}

void fb_paula_sla::init(fb_abs_master & mas)
{
	this->mas = &mas;
}

void fb_paula_sla::fb_set_cyc(fb_cyc_action cyc)
{
	if (cyc == stop) {
		state = idle;
	}
	else {
		if (we)
			state = actwaitwr;
		else if (mas)
		{
			mas->fb_set_D_rd(paula.read_regs(addr));
			mas->fb_set_ACK(ack);
		}
	}

}

uint8_t fb_paula_sla::peek(uint32_t addr) {
    return paula.read_regs(addr);
}

void fb_paula_sla::poke(uint32_t addr, uint8_t dat) {
    paula.write_regs(addr, dat);
}


void fb_paula_sla::fb_set_A(uint32_t addr, bool we)
{
	this->addr = addr;
	this->we = we;

}

void fb_paula_sla::fb_set_D_wr(uint8_t dat)
{
	if (state == actwaitwr) {
		paula.write_regs(addr, dat);
		mas->fb_set_ACK(ack);
		state = idle;
	}


}

void fb_paula_sla::reset()
{
	state = idle;
}

void fb_paula_mas::init(fb_abs_slave & sla)
{
	this->sla = &sla;
	reset();
}

void fb_paula_mas::fb_set_ACK(fb_ack ack)
{
	state = idle;
	if (sla)
		sla->fb_set_cyc(stop);
}

void fb_paula_mas::fb_set_D_rd(uint8_t dat)
{
	if (state == act) {
		paula.gotByte(cur_chan, dat);
	}
}

void fb_paula_mas::reset()
{
	state = idle;
}

bool fb_paula_mas::getByte(uint32_t addr, uint8_t channel)
{
	if (state == idle) {
		if (sla) {
			cur_addr = addr;
			cur_chan = channel;
			state = act;
			sla->fb_set_A(addr, 0);
			sla->fb_set_cyc(start);
		}
		else {
			paula.gotByte(channel, 0);
		}
		return true;
	}

	return false;
}

void fb_paula::sound_fillbuf(int16_t *buffer, int len) {
	if (outbufempty)
	{
		if (outbuf.size() > PAULA_HYSTER)
			outbufempty = false;
	}
	else if (outbuf.size() >= len) {
		for (int i = 0; i < len ; i ++) {
			*buffer++ += outbuf.front();
			outbuf.pop();
		}
	} else {
		outbufempty = true;
		std::cerr << "psnd:rd:empty\n";
	}
}

