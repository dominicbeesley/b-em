#ifndef ABS_MASTER_H
#define ABS_MASTER_H

/**********************************************

Defines a base abstract class for the behaviour
of a blitter bus master

The external logic should only call tick with
exec = true when the slaves cycle is completed

The external bus should only  

**********************************************/


#pragma once

#include "fb_fishbone.h"
#include "fb_abs_tickable.h"

class fb_abs_slave;

class fb_abs_master
{

public:
	virtual void init(fb_abs_slave &sla) = 0;

	virtual void fb_set_ACK(fb_ack ack) = 0;
	virtual void fb_set_D_rd(uint8_t dat) = 0;
};


#endif //ABS_MASTER_H