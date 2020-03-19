#ifndef _FB_DEF_SLAVE_H_
#define _FB_DEF_SLAVE_H_
#pragma once

#include "fb_abs_slave.h"
#include "fb_abs_master.h"

class fb_def_slave : public fb_abs_slave
{
public:

	// Inherited via fb_abs_slave
	virtual void init(fb_abs_master & mas) override;
	virtual void fb_set_cyc(fb_cyc_action cyc) override;
	virtual void fb_set_A(uint32_t addr, bool we) override;
	virtual void fb_set_D_wr(uint8_t dat) override;

private:
	fb_abs_master *mas;
};

#endif // !_FB_DEF_SLAVE_H_
