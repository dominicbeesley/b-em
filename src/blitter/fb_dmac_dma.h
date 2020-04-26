#ifndef _FB_DMAC_DMA_H_
#define _FB_DMAC_DMA_H_

#include "fb_abs_resettable.h"
#include "fb_abs_tickable.h"
#include "fb_abs_master.h"
#include "fb_abs_slave.h"

#define DMA_CHANNELS 4

typedef enum {
	A_CTL = 0,
	A_SRC_ADDR_BANK = 1,
	A_SRC_ADDR = 2,
	A_DEST_ADDR_BANK = 4,
	A_DEST_ADDR = 5,
	A_COUNT = 7,
	A_DATA = 9,
	A_CTL2 = 10,
	A_PAUSE_VAL = 11,
	A_SEL = 15
} A_DMA;

class fb_dmac_dma;
class blitter_top;

class fb_dma_sla : public fb_abs_slave, public fb_abs_resettable {

public:

	fb_dma_sla(fb_dmac_dma& _dma) : dma(_dma) {};

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
	fb_dmac_dma& dma;
	fb_abs_master* mas; // connected master

	enum fb_dma_sla_state { idle, actwaitwr } state;
	uint32_t addr;
	bool we;



};

class fb_dma_mas : public fb_abs_master, public fb_abs_resettable {

public:

	fb_dma_mas(fb_dmac_dma &_dma) : dma(_dma) {};

	// Inherited via fb_abs_master
	virtual void init(fb_abs_slave & sla) override;
	virtual void fb_set_ACK(fb_ack ack) override;
	virtual void fb_set_D_rd(uint8_t dat) override;
	// Inherited via fb_abs_resettable
	virtual void reset() override;

	friend fb_dmac_dma;

protected:

	enum fb_paula_mas_state { idle, actrd, actwr } state;

	bool get_byte(uint32_t addr, uint8_t channel);
	bool set_byte(uint32_t addr, uint8_t data);

private:
	fb_dmac_dma& dma;
	fb_abs_slave* sla;
	// connected slave

	uint32_t cur_addr;
	uint8_t cur_chan;
};

#pragma once
class fb_dmac_dma :	public fb_abs_tickable
{
public:

	enum dma_state_type {sIdle, sStart, sMemAccSRC, sMemAccSRC2, sMemAccPAUSE, sMemAccDEST, sMemAccDEST2, sFinish} ;
	enum step_type {step_none, step_up, step_down, step_nop} ;
	enum stepsize_type {stepsize_byte, stepsize_word, stepsize_wordswapdest, stepsize_wordswapsrc} ;

	typedef 	struct
	{
		//user facing registers
		bool		ctl_act;
		step_type	ctl_step_src;
		step_type	ctl_step_dest;
		bool		ctl_halt;
		bool		ctl_extend;

		bool		ctl2_pause;
		bool		ctl2_if;
		bool		ctl2_ie;
		stepsize_type	ctl2_stepsize;

		uint8_t		src_bank;
		uint16_t	src_addr;
		uint8_t		dest_bank;
		uint16_t	dest_addr;

		uint8_t		dat_l;
		uint8_t		dat_h;
		uint16_t	count;

		uint8_t		pause_val;

		//other state registers
		bool		count_finish;
		bool		count_clken;

		uint8_t		pause_ct_dn;
		bool		pause_ct_dn_finished;

		bool		data_started; //indicates a read/write has been initiated
		bool		data_ready;	//set when data received in src states
		dma_state_type state;


	} dma_channelreg_t;
	
	fb_dmac_dma(blitter_top &_top) : sla(*this), mas(*this), top(_top) {};


	// Inherited via fb_abs_tickable
	virtual void reset() override;
	virtual void tick(bool sys) override;
	virtual void init();

	fb_dma_sla& getSlave() { return sla; }
	fb_dma_mas& getMaster() { return mas; }

	friend fb_dma_mas;
	friend fb_dma_sla;


protected:

	void update_cpu();

	uint8_t				channel_sel;

	dma_channelreg_t	channel_regs[DMA_CHANNELS];

	void byte_got(uint8_t channel, int8_t dat); // called by master when byte received
	void write_regs(uint8_t addr, uint8_t dat); // called by slave when a write happens
	uint8_t read_regs(uint8_t addr); //called by slave when a read happens


	fb_dma_sla sla;
	fb_dma_mas mas;
	blitter_top& top;

	step_type to_step(uint8_t s) {
		switch (s & 0x03) {
		case 0x00:
			return step_none;
		case 0x01:
			return step_up;
		case 0x02:
			return step_down;
		default:
			return step_nop;

		}
	}

	stepsize_type to_stepsize(uint8_t s) {
		switch (s & 0x03) {
		case 0x01:
			return stepsize_word;
		case 0x02:
			return stepsize_wordswapdest;
		case 0x03:
			return stepsize_wordswapsrc;
		default:
			return stepsize_byte;

		}
	}

	uint8_t step_to_int(step_type s) {
		switch (s) {
		case step_up:
			return 1;
		case step_down:
			return 2;
		case step_nop:
			return 3;
		default:
			return 0;
		}
	}

	uint8_t stepsize_to_int(stepsize_type ss) {
		switch (ss) {
		case stepsize_word:
			return 1;
		case stepsize_wordswapdest:
			return 2;
		case stepsize_wordswapsrc:
			return 3;
		default:
			return 0;
		}
	}
};

#endif 

