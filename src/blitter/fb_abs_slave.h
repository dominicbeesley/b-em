#ifndef _FB_ABS_SLAVE_H_
#define _FB_ABS_SLAVE_H_

#pragma once

#include "fb_abs_tickable.h"
#include "fb_fishbone.h"

class fb_abs_master;

class fb_abs_slave
{
public:
	virtual void init(fb_abs_master &mas) = 0;

	virtual void fb_set_cyc(fb_cyc_action cyc) = 0;
	virtual void fb_set_A(uint32_t addr, bool we) = 0;
	virtual void fb_set_D_wr(uint8_t dat) = 0;

};


#endif //_FB_ABS_SLAVE_H_