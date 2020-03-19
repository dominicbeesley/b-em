#ifndef _FB_JIMCTRL_H_
#define _FB_JIMCTRL_H_

#pragma once
#include "fb_abs_slave.h"

class blitter_top;

class fbsla_jimctrl :public fb_abs_slave, fb_abs_resettable
{
public:
	fbsla_jimctrl(blitter_top& _top) : top(_top) {};


	// Inherited via fb_abs_slave
	virtual void init(fb_abs_master & mas) override;
	virtual void fb_set_cyc(fb_cyc_action cyc) override;
	virtual void fb_set_A(uint32_t addr, bool we) override;
	virtual void fb_set_D_wr(uint8_t dat) override;

	// Inherited via fb_abs_resettable
	virtual void reset() override;

protected:
	blitter_top& top;
	fb_abs_master* mas;

	enum fb_mem_chipram_state { idle, actwaitwr } state;
	uint32_t addr;
	bool we;
};

#endif // !_FB_JIMCTRL_H_

