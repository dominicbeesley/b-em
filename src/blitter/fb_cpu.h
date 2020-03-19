#ifndef _FB_CPU_H_
#define _FB_CPU_H_
#pragma once

#include "fb_fishbone.h"
#include "fb_abs_tickable.h"
#include "fb_abs_master.h"
#include "m6502.h"

class blitter_top;

class fb_cpu : public fb_abs_tickable, public fb_abs_master {
public:

	fb_cpu(blitter_top &_top) 
		: top(_top)
	{
		cpu.init();
	}

	virtual void reset() override;
	virtual void tick(bool sys) override;
	virtual void init(fb_abs_slave & sla) override;
	virtual void fb_set_ACK(fb_ack ack) override;
	virtual void fb_set_D_rd(uint8_t dat) override;


	void execute_set_input(int inputnum, int state) { cpu.execute_set_input(inputnum, state); };

protected:



	enum cpu_state { idle, act, acked, dotick } state;

	m6502_device cpu;

	uint32_t physAddr;

	blitter_top &top;

private:
	fb_abs_slave *sla;
	uint8_t D_rd;
};

#endif // !_FB_CPU_H_

