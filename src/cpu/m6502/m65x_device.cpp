
#include "m65x_device.h"


void m65x_device::start()
{
	init();
}

void m65x_device::reset()
{

	sync = false;
	RNW = true;
}

int bob() {
	return 1;
}



