// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

	devcpu.h

	CPU device definitions.

***************************************************************************/
#ifndef BLITTER_TOP_H
#define BLITTER_TOP_H

#pragma once

#include "devcpu.h"
#include "m65x_device.h"
#include "m65c02.h"
#include "fb_cpu.h"
#include "fb_sys.h"
#include "fb_intcon.h"
#include "fbsla_mem_chipram.h"
#include "fbsla_mem_flash.h"
#include "fbsla_jimctrl.h"
#include "fbsla_memctl.h"
#include "fb_paula.h"
#include "fb_dmac_dma.h"
#include "fb_blitter.h"

typedef enum blit_SLAVE_NO {
	SLAVE_NO_JIMCTL,
	SLAVE_NO_SYS,
	SLAVE_NO_CHIPRAM,
	SLAVE_NO_CHIPFLASH,
	SLAVE_NO_MEMCTL,
	SLAVE_NO_EEPROM,
	SLAVE_NO_DMA,
	SLAVE_NO_BLIT,
	SLAVE_NO_SOUND,
	SLAVE_NO_AERIS,
	SLAVE_NO__COUNT
} blit_SLAVE_NO;

typedef enum blit_MAS_NO {
	MAS_NO_CPU,
	MAS_NO_PAULA,
	MAS_NO_DMA,
	MAS_NO_BLIT,
	MAS_NO_COUNT
} blit_MAS_NO;

//level no's used for irq / nmis
typedef enum {
	compno_SYS,
	compno_DMA,
	compno_COUNT,
	compno_BLIT
} compno_t;

typedef uint8_t (*readmem_t)(uint16_t addr);
typedef void (*writemem_t)(uint16_t addr, uint8_t dat);

class blitter_top : public m65x_device {
public:

	blitter_top(readmem_t rdmem, writemem_t wrmem) :
		m65x_device(),
		sys(*this),
		cpu(*this),
		jimctl(*this),
		memctl(*this),
		intcon(*this, MAS_NO_COUNT, SLAVE_NO__COUNT),
		paula(),
		dma(*this),
		blitter(*this),
        sys_readmem(rdmem),
        sys_writemem(wrmem)
	{
		powerReset();
		reset();
	};

	void init() override;
	bool tick() override;
	bool tick(int skip);
	void reset() override;


	//6502 interrupt etc these are strictly for use by SYS
	virtual void execute_set_input(int inputnum, int state) override;

	//set halt / irq / nmi from components other than sys
	//note nmi is edge triggered
	void set_interrupts(compno_t comp_no, bool halt, bool irq, bool nmi);
	
	friend class fb_sys;

	void set_ROMPG(uint8_t d) {
		reg_ROMPG = d;
	};
	uint8_t get_ROMPG() { return reg_ROMPG; };
	void set_JIMEN(bool b) {
		reg_jimEn = b;
	};
	bool get_JIMEN() {
		return reg_jimEn;
	};
	void set_JIMPAGE_L(uint8_t d) {
		reg_jimPage = (reg_jimPage & 0xFF00) | (uint16_t)d;
	}
	void set_JIMPAGE_H(uint8_t d) {
		reg_jimPage = (reg_jimPage & 0xFF) | ((uint16_t)d) << 8;
	}
	uint8_t get_JIMPAGE_L() { return (uint8_t)reg_jimPage; }
	uint8_t get_JIMPAGE_H() { return (uint8_t)(reg_jimPage >> 8); }
	uint16_t get_JIMPAGE() { return reg_jimPage; }

	uint32_t log2phys(uint32_t ain);
	blit_SLAVE_NO addr2slaveno(uint32_t addr);

	bool get_cfg_swram_en() { return true;}
	bool get_nioice_debug_shadow() { return false; }
	bool get_swmos_shadow() { return false; }


	uint8_t get_blturbo() { return reg_blturbo; }
	void set_blturbo(uint8_t val) { reg_blturbo = val; }

	void powerReset();

	void setIRQ(int levelno, bool assert);
	void setNMI(int levelno, bool assert);
	void setHALT(int levelno, bool assert);

	void sound_fillbuf(int16_t *buffer, int len);

    long count_ticks = 0;

    m6502_device* get_cpu() { return cpu.get_cpu(); }

    uint8_t peek(uint16_t addr);
    void poke(uint16_t addr, uint8_t dat);

protected:

    readmem_t sys_readmem;
    writemem_t sys_writemem;

	uint32_t bits_halt; //the opposite of RDY
	uint32_t bits_irq; 
	uint32_t bits_nmi;

	fb_sys sys;
	fb_cpu cpu;
	fbsla_mem_chipram chipram;
	fbsla_mem_flash flash;
	fbsla_jimctrl jimctl;
	fbsla_memctl memctl;
	fb_paula paula;
	fb_dmac_dma dma;
	fb_blitter blitter;

	fb_intcon intcon;

	uint8_t reg_ROMPG;
	bool	reg_jimEn;
	uint16_t reg_jimPage;
	uint8_t reg_blturbo;

	void tick_int(bool sys);
};



#endif //BLITTER_TOP_H
