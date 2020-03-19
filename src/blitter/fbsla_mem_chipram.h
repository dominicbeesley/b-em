#ifndef _FB_MEM_CHIPRAM_H_
#define _FB_MEM_CHIPRAM_H_

#include "fb_abs_slave.h"
#include "fb_abs_master.h"

#pragma once
class fbsla_mem_chipram : public fb_abs_slave, public fb_abs_resettable
{
public:

	enum fb_mem_chipram_state { idle, actwaitwr } state;

	// Inherited via fb_abs_slave
	virtual void init(fb_abs_master & mas) override;
	virtual void fb_set_cyc(fb_cyc_action cyc) override;
	virtual void fb_set_A(uint32_t addr, bool we) override;
	virtual void fb_set_D_wr(uint8_t dat) override;

	// Inherited via fb_abs_resettable
	virtual void reset() override;


private:
	uint8_t mem[0x200000];

	bool we;
	uint32_t addr;

	fb_abs_master* mas;


};

#endif //!_FB_MEM_CHIPRAM_H_