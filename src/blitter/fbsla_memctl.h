#ifndef _FBSLA_MEMCTL_H_
#define _FBSLA_MEMCTL_H_


#pragma once
#include "fb_abs_slave.h"

class blitter_top;

class fbsla_memctl : public fb_abs_slave, public fb_abs_resettable
{
public:
	fbsla_memctl(blitter_top& _top);
	~fbsla_memctl();

	enum fb_mem_chipram_state { idle, actwaitwr } state;


	// Inherited via fb_abs_resettable
	virtual void reset() override;

	// Inherited via fb_abs_slave
	virtual void init(fb_abs_master & mas) override;
	virtual void fb_set_cyc(fb_cyc_action cyc) override;
	virtual void fb_set_A(uint32_t addr, bool we) override;
	virtual void fb_set_D_wr(uint8_t dat) override;

private: 
	blitter_top& top;
	bool we;
	uint32_t addr;
	uint8_t D_wr;
	fb_abs_master* mas;
};


#endif //!_FBSLA_MEMCTL_H_