#ifndef _FB_MEM_FLASH_H_
#define _FB_MEM_FLASH_H_

#include "fb_abs_slave.h"
#include "fb_abs_master.h"

#pragma once
class fbsla_mem_flash : public fb_abs_slave, public fb_abs_tickable
{
public:

	enum fb_mem_flash_state { idle, actwaitwr } state;

	enum flash_prog_state { 
		///normal read state
		fp_normal = 0, 
		///software id mode
		fp_softid = 1, 
		///have received 1st prog. 5555/AA
		fp_p1 = 2, 
		///have received 2nd prog. 2AAA/55
		fp_p2 = 3, 
		///have received 3rd prog. 5555/*0 store data top nybble as op
		fp_p3 = 4,
		///have received 4th prog. go to byte program, soft id or wait for further erase commands either BA/DD or 5555/AA
		fp_p4 = 5, 
		///have received 5th prog. (erase) 2AAA/55
		fp_p5 = 6, 
		///have received 6th prog. (erase) SA/30 or 5555/10
		fp_p6 = 7, 
		///do prog i.e. toggle D6, invert D7 until countdown
		fp_prog = 10,
	} prog_state;

	// Inherited via fb_abs_slave
	virtual void init(fb_abs_master & mas) override;
	virtual void fb_set_cyc(fb_cyc_action cyc) override;
	virtual void fb_set_A(uint32_t addr, bool we) override;
	virtual void fb_set_D_wr(uint8_t dat) override;

	// Inherited via fb_abs_resettable
	virtual void reset() override;

	// Inherited via fb_abs_tickable
	virtual void tick(bool sys) override;

    virtual uint8_t peek(uint32_t addr) override;
    virtual void poke(uint32_t addr, uint8_t dat);


private:
	uint8_t mem[0x80000];

	uint8_t prog_op;
	uint32_t prog_addr;
	bool prog_toggle;
	uint32_t prog_ct_dn;

	bool we;
	uint32_t addr;

	fb_abs_master* mas;

	const uint8_t manu_id = 0xBF;
	const uint8_t dev_id = 0xD6;



};

#endif //!_FB_MEM_FLASH_H_
