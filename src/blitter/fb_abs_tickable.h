#ifndef _FB_TICKABLE_H
#define _FB_TICKABLE_H

#include "fb_abs_resettable.h"

#pragma once
class fb_abs_tickable : public fb_abs_resettable
{
public:
	//called at the end of each cycle, sys indicates this is aligned with 2/1 MHz BBC cycle
	virtual void tick(bool sys) = 0;
};


#endif //_FB_TICKABLE_H

