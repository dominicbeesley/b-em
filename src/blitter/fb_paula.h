#ifndef _FB_PAULA_H_
#define _FB_PAULA_H_

#pragma once


#include "fb_abs_tickable.h"
#include "fb_abs_master.h"
#include "fb_abs_slave.h"

#include <vector>
#include <queue>


/* Much simplified compared to real hardware:

	- single master interface and arbitration done here
	- combined slave interface for all channels and global registers

*/

class fb_paula;

#define NUM_CHANNELS	  4
#define PAULA_STREAM_RATE 31250
#define PAULA_HYSTER	2000
#define PAULA_BUFLENMAX 2000*3

#define PAULA_PAULA_CLK 3547672
#define PAULA_RAM_CLK   8000000
#define PAULA_CYCLES_PER_STREAM_SAMPLE PAULA_RAM_CLK/PAULA_STREAM_RATE
#define PAULA_PCLK_LIM  65536
#define PAULA_PCLK_A    (int)((((long long)PAULA_PCLK_LIM) * ((long long)PAULA_PAULA_CLK)) / (long long)PAULA_RAM_CLK)


typedef struct
{
	//user facing registers
	int8_t		data;
	uint8_t		addr_bank;
	uint16_t	addr;

	uint8_t		period_h_latch;
	uint16_t	period;
	uint16_t	len;
	bool		act;
	bool		repeat;
	uint8_t		vol;
	uint16_t	repoff;
	uint8_t		peak;

	//internal registers
	bool		clken_sam_next;
	bool		act_prev;
	uint16_t	samper_ctr;
	uint8_t		data_next;
	uint16_t	sam_ctr;

} paula_CHANNELREGS;



class fb_paula_sla : public fb_abs_slave, public fb_abs_resettable {

public:

	fb_paula_sla(fb_paula& _paula) : paula(_paula) {};

	// Inherited via fb_abs_slave
	virtual void init(fb_abs_master & mas) override;
	virtual void fb_set_cyc(fb_cyc_action cyc) override;
	virtual void fb_set_A(uint32_t addr, bool we) override;
	virtual void fb_set_D_wr(uint8_t dat) override;
	// Inherited via fb_abs_resettable
	virtual void reset() override;

    virtual uint8_t peek(uint32_t addr) override;
    virtual void poke(uint32_t addr, uint8_t dat);


private:
	fb_paula& paula;
	fb_abs_master* mas; // connected master

	enum fb_paula_sla_state { idle, actwaitwr } state;
	uint32_t addr;
	bool we;



};

class fb_paula_mas : public fb_abs_master, public fb_abs_resettable {

public:

	fb_paula_mas(fb_paula &_paula) : paula(_paula) {};

	// Inherited via fb_abs_master
	virtual void init(fb_abs_slave & sla) override;
	virtual void fb_set_ACK(fb_ack ack) override;
	virtual void fb_set_D_rd(uint8_t dat) override;
	// Inherited via fb_abs_resettable
	virtual void reset() override;

	friend fb_paula;

protected:

	enum fb_paula_mas_state {idle, act} state;

	bool getByte(uint32_t addr, uint8_t channel);

private:
	fb_paula& paula;
	fb_abs_slave* sla;
	// connected slave

	uint32_t cur_addr;
	uint8_t cur_chan;
};

class fb_paula : public fb_abs_tickable
{
public:
	fb_paula() : sla(*this), mas(*this) {};
	virtual ~fb_paula();

	// Inherited via fb_abs_tickable
	virtual void reset() override;
	virtual void tick(bool sys) override;
	virtual void init();

	fb_paula_sla& getSlave() { return sla; }
	fb_paula_mas& getMaster() { return mas; }

	friend fb_paula_mas;
	friend fb_paula_sla;

	void sound_fillbuf(int16_t *buffer, int len);

protected:
	void gotByte(uint8_t channel, int8_t dat);

private:
	fb_paula_sla sla;
	fb_paula_mas mas;

	paula_CHANNELREGS	ChannelRegs[NUM_CHANNELS];
	uint8_t				ChannelSel;
	uint8_t				Volume;	//paula volume

	uint32_t sample_clock_acc = 0; //sample clock accumulator - when this overflows do a sound sample
	uint32_t paula_clock_acc = 0; //paula clock acumulator - when this overflows do a Paula period

	void write_regs(uint8_t addr, uint8_t dat);
	uint8_t read_regs(uint8_t addr);

	std::queue<uint16_t> outbuf;
	bool outbufempty = true;

};


#endif //!_FB_PAULA_H_
