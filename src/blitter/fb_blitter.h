#ifndef _FB_BLITTER_H_
#define _FB_BLITTER_H_

#pragma once

//TODO: aeries halt input


#include "fb_abs_tickable.h"
#include "fb_abs_master.h"
#include "fb_abs_slave.h"

#include <vector>
#include <queue>


/* Much simplified compared to real hardware:

	- single master interface and arbitration done here
	- combined slave interface for all channels and global registers
*/

const int A_BLIT_BASE = 0x60;
const int A_BLITOFFS_BLTCON 	= 0x00;
const int A_BLITOFFS_FUNCGEN 	= 0x01;
const int A_BLITOFFS_WIDTH 	= 0x02;
const int A_BLITOFFS_HEIGHT 	= 0x03;
const int A_BLITOFFS_SHIFT 	= 0x04;
const int A_BLITOFFS_MASK_FIRST = 0x05;
const int A_BLITOFFS_MASK_LAST 	= 0x06;
const int A_BLITOFFS_DATA_A 	= 0x07;
const int A_BLITOFFS_ADDR_A 	= 0x08;
const int A_BLITOFFS_DATA_B 	= 0x0B;
const int A_BLITOFFS_ADDR_B 	= 0x0C;
const int A_BLITOFFS_ADDR_C 	= 0x0F;
const int A_BLITOFFS_ADDR_D 	= 0x12;
const int A_BLITOFFS_ADDR_E 	= 0x15;
const int A_BLITOFFS_STRIDE_A 	= 0x18;
const int A_BLITOFFS_STRIDE_B 	= 0x1A;
const int A_BLITOFFS_STRIDE_C 	= 0x1C;
const int A_BLITOFFS_STRIDE_D 	= 0x1E;
const int A_BLITOFFS_ADDR_D_MIN = 0x40;
const int A_BLITOFFS_ADDR_D_MAX = 0x43;

const uint16_t BLIT_STRIDE_MASK = 0x07FF;
const int BLIT_STRIDE_HI_BIT = 11;

class fb_blitter;
class blitter_top;

class fb_blitter_sla : public fb_abs_slave, public fb_abs_resettable {

public:

	fb_blitter_sla(fb_blitter& _blitter) : blitter(_blitter) {};

	// Inherited via fb_abs_slave
	virtual void init(fb_abs_master & mas) override;
	virtual void fb_set_cyc(fb_cyc_action cyc) override;
	virtual void fb_set_A(uint32_t addr, bool we) override;
	virtual void fb_set_D_wr(uint8_t dat) override;
	// Inherited via fb_abs_resettable
	virtual void reset() override;

    virtual uint8_t peek(uint32_t addr) override;
    virtual void poke(uint32_t addr, uint8_t dat) override;


private:
	fb_blitter& blitter;
	fb_abs_master* mas; // connected master

	enum fb_blitter_sla_state { idle, actwaitwr } state;
	uint32_t addr;
	bool we;
};

class fb_blitter_mas : public fb_abs_master, public fb_abs_resettable {

public:

	fb_blitter_mas(fb_blitter &_blitter) : blitter(_blitter) {};

	// Inherited via fb_abs_master
	virtual void init(fb_abs_slave & sla) override;
	virtual void fb_set_ACK(fb_ack ack) override;
	virtual void fb_set_D_rd(uint8_t dat) override;
	// Inherited via fb_abs_resettable
	virtual void reset() override;

	friend fb_blitter;


protected:

	enum fb_blitter_mas_state {idle, act, act_wr} state;

	bool getByte(uint32_t addr);
	bool setByte(uint32_t addr, uint8_t dat);

private:
	fb_blitter& blitter;
	fb_abs_slave* sla;
	// connected slave

	uint32_t cur_addr;
};


class fb_blitter : public fb_abs_tickable
{
public:
	fb_blitter(blitter_top &_top) : top(_top), sla(*this), mas(*this) {};
	virtual ~fb_blitter();

	// Inherited via fb_abs_tickable
	virtual void reset() override;
	virtual void tick(bool sys) override;
	virtual void init();

	fb_blitter_sla& getSlave() { return sla; }
	fb_blitter_mas& getMaster() { return mas; }

	friend fb_blitter_mas;
	friend fb_blitter_sla;

	enum class blitter_bppmode {
		bpp_1 = 0x00,
		bpp_2 = 0x01,
		bpp_4 = 0x02,
		bpp_8 = 0x03
	};


protected:
	void gotByte(int8_t dat);

	enum class state_type {
		sIdle		// waiting for act to be set
	,	sStart		// dummy cycle before starting (TODO: get rid?)
	,	sLineCalc	// when drawing lines calculate what needs to happen for/after this pixel
	,	sMemAccA	// get channel A
	,	sMemAccB	// get channel B
	,	sMemAccC	// get channel C data
	,	sMemAccD	// write channel D data
	,	sMemAccC_min	// line only - second C exec to move minor axis
	,	sMemAccD_min	// line only - second D exec to move minor axis
	,	sMemAccE	// channel E write
	,	sMemAccPend	// NOT IN FPGA - a read/write is being blocked in the master try again next tick
	,  sFinish
	};

	enum class state_cha_A {
		sMemAccA, sShiftA1, sShiftA2, sShiftA3, sShiftA4, sShiftA5, sShiftA6, sShiftA7
	};

	enum class blit_addr_direction {
		cha_e		// just increment address always
	,  	plot_up		// cell:	subtract 1 or bytes_stride and set lower 3 bits if crossing a cell boundary
				// notcell:	subtract bytes_stride 
	,  	plot_down	// cell:	add 1 or bytes_stride and reset lower 3 bits if crossing a cell boundary
				// notcell:	add bytes stride 
	,	plot_right	// cell:	add 8
				// notcell:	add 1
	,	plot_left	// cell:	sub 8
				// notcell:	sub 8
	,	spr_wrap	// cell:	add bytes_stride - width*8 - 7 when at end of cell (i.e. lower 3 bits set) else
				// 		add 1 - width * 8
				// notcell:	add bytes_stride - width
	,  	none		// do nothing
	};


	/* state registers */

	state_type r_blit_state;
	state_type r_blit_state_pend; // used in sMemAccPend state
	state_cha_A r_accA_state_cur;


	uint8_t r_line_pxmaskcur;
	bool r_clken_addr_calc_start;
	bool r_data_ready;
	uint8_t r_data_read;
	bool r_cha_A_last_mask;
	bool r_cha_A_first;
	uint8_t r_y_count;
	bool r_line_minor;
	uint8_t r_row_countdn;


	/* user accessible registers */
	// bltcon/act

	bool r_BLTCON_act;		// bit 7	-- write as 1
	bool r_BLTCON_cell;		// bit 6 channels C,D are in CELL addressing mode
	blitter_bppmode r_BLTCON_mode;	// bit 5,4 00 = 1bpp , 01 = 2bpp, 10 = 4bpp, 11 = 8bpp
	bool r_BLTCON_line;		// bit 3 - when set line drawing is active
	bool r_BLTCON_collision;	// bit 2 - reset if any non-zero D is detected
	bool r_BLTCON_wrap;		// bit 1 - wrap C/D addresses

	// bltcon/cfg
	bool r_BLTCON_execE;		// bit 4
	bool r_BLTCON_execD;		// bit 3
	bool r_BLTCON_execC;		// bit 2
	bool r_BLTCON_execB;		// bit 1
	bool r_BLTCON_execA;		// bit 0

	uint8_t r_FUNCGEN;

	uint8_t	r_width;
	uint8_t	r_height;

	uint8_t r_cha_A_data;
	uint8_t r_cha_A_data_pre;		//only 6 bits in fpga
	uint8_t r_cha_B_data;
	uint8_t r_cha_B_data_pre;		//only 6 bits in fpga
	uint8_t r_cha_C_data;
	uint8_t i_cha_D_data;			//note not a register
	uint32_t r_cha_A_addr;			//note only 24 bits in fpga
	uint32_t r_cha_B_addr;			//note only 24 bits in fpga
	uint32_t r_cha_C_addr;			//note only 24 bits in fpga
	uint32_t r_cha_D_addr;			//note only 24 bits in fpga
	uint32_t r_cha_D_addr_min;		//note only 24 bits in fpga
	uint32_t r_cha_D_addr_max;		//note only 24 bits in fpga
	uint32_t r_cha_E_addr;			//note only 24 bits in fpga

	//note all the stride values are actually only 12 bits, except for A which is used for line
	//slope	
	int16_t r_cha_A_stride; 		// stride A needs to be at least 16 bit for line slopes
	int16_t r_cha_B_stride;
	int16_t r_cha_C_stride;
	int16_t r_cha_D_stride;

	// actuall only 3 bits
	uint8_t r_shift_A;
	uint8_t r_shift_B;

	uint8_t	r_mask_first;
	uint8_t r_mask_last;


	// this is clocked by the fast clock in the fpga, just calc instantly here
	inline uint16_t blit_addr_next(
		blit_addr_direction 	direction,
		bool			mode_cell,
		int16_t			bytes_stride,
		uint8_t			width,
		bool			wrap,
		uint16_t		addr_in,
		uint16_t		addr_min,
		uint16_t		addr_max
	);
	const char *dirstr(blit_addr_direction a);

	inline blit_addr_direction fnDirection(bool spr_last, bool line_major);
	inline uint8_t get_chaA_shifted1();
	inline uint8_t get_chaA_masked();
	inline uint8_t get_chaA_shifted2();
	inline uint8_t get_chaA_explode();
	inline uint8_t get_chaA_width_in_bytes();
	inline uint8_t get_chaB_shifted();
	inline uint8_t get_funcgen_data();
	inline state_type get_state_next();
	inline state_cha_A get_accA_state_next();
	inline bool get_main_last();
	inline bool get_cha_A_last();
	inline uint16_t get_next_addr_blit();
private:
	fb_blitter_sla sla;
	fb_blitter_mas mas;
	blitter_top& top;


	void write_regs(uint8_t addr, uint8_t dat);
	uint8_t read_regs(uint8_t addr);

	inline void STRIDE_HI_SET(int16_t &reg, uint8_t v) {

		v &= (BLIT_STRIDE_MASK >> 8);
		//sign extend
		if (v & (1 << BLIT_STRIDE_HI_BIT))
			v |= ~(BLIT_STRIDE_MASK);

		reg = ((reg & 0x00FF) | ((uint16_t)v) << 8);
	}
	inline uint8_t STRIDE_HI_GET(uint16_t v) {
		return (v & BLIT_STRIDE_MASK) >> 8;
	}

	inline void STRIDE_LO_SET(int16_t &reg, uint8_t v) {
		reg = ((reg & 0xFF00) | ((uint16_t)v));
	}
	inline void ADDR_BANK_SET(uint32_t &reg, uint8_t v) {
		 reg = reg & 0x00FFFF | ((uint32_t)v) << 16;
	}
	inline void ADDR_HI_SET(uint32_t &reg, uint8_t v) {
		 reg = reg & 0xFF00FF | ((uint32_t)v) << 8;
	}
	inline void ADDR_LO_SET(uint32_t &reg, uint8_t v) {
		 reg = reg & 0xFFFF00 | ((uint32_t)v) << 0;
	}

	inline uint8_t BANK8(uint32_t v) {
		return v >> 16;
	}
	inline uint8_t HI8(uint32_t v) {
		return v >> 8;
	}
	inline uint8_t LO8(uint32_t v) {
		return v;
	}

	inline void ADDR_16_SET(uint32_t &reg, uint16_t val) {
		reg = (reg & 0xFF0000) | val;
	}

};


#endif //!_FB_BLITTER_H_
