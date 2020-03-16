// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

	m65x_device.h

	BASE MOS Technology 65xx

	This is also a base class for other things that sit in the 6502 cpu 
	socket i.e. the blitter board

***************************************************************************/

#ifndef MAME_CPU_M65X_DEVICE_H
#define MAME_CPU_M65X_DEVICE_H

#include <stdint.h>
#include "devcpu.h"

// I/O line definitions
enum
{
	// input lines
	INPUT_LINE_IRQ0,
	INPUT_LINE_NMI,
	INPUT_LINE_V,

	// special input lines that are implemented in the core
	INPUT_LINE_RESET,
	INPUT_LINE_HALT,
	MAX_INPUT_LINES
};



class m65x_device;


class m65x_device : public cpu_device {
public:

	//typedef void(*StatFn)(m65x_device &cpu);
	typedef void *StatFn;

	enum {
		IRQ_LINE = INPUT_LINE_IRQ0,
		NMI_LINE = INPUT_LINE_NMI,
		HALT_LINE = INPUT_LINE_HALT,
		V_LINE = INPUT_LINE_V
	};

	bool get_sync() const { return sync; }
	uint16_t getADDR() const { return ADDR; }
	uint8_t getDATA() const { return DAT; }
	bool getRNW() const { return RNW; }
	void setDATA(uint8_t val) { DAT = val; }
	


	m65x_device(void) : cpu_device(),
		//sync_w(*this),
		DAT(0),
		sync(false),
		ADDR(0),
		RNW(true)
	{};

	virtual void init() {

		RNW = true;
		sync = false;
		ADDR = 0;
		DAT = 0;
	};

	virtual void start() override;
	virtual void reset() override;


protected:



	uint16_t	ADDR;
	uint8_t		DAT;
	bool		RNW;
	bool		sync;

	//should really be abstract but then blitter which doesn't have the concept would have to override
	virtual void postfetch_int() {}

};

enum {
	M6502_IRQ_LINE = m65x_device::IRQ_LINE,
	M6502_NMI_LINE = m65x_device::NMI_LINE,
	M6502_SET_OVERFLOW = m65x_device::V_LINE
};



#endif // MAME_CPU_M65X_DEVICE_H
