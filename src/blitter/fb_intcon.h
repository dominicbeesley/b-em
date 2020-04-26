#ifndef _FB_INTCON_H_
#define _FB_INTCON_H_


#pragma once
#include "fb_fishbone.h"
#include "fb_abs_master.h"
#include "fb_abs_slave.h"
#include <vector>
#include <stdlib.h>
#include <stdexcept>


using namespace std;

class blitter_top;
class fb_intcon;
class fb_intcon_mas;

class fb_intcon_sla : public fb_abs_slave, public fb_abs_resettable {
public:
	fb_intcon_sla(fb_intcon& _intcon) :
		intcon(_intcon), mas(0)  {
		reset();
	}

	// Inherited via fb_abs_slave
	virtual void init(fb_abs_master & _mas) override;
	virtual void fb_set_cyc(fb_cyc_action cyc) override;
	virtual void fb_set_A(uint32_t addr, bool we) override;
	virtual void fb_set_D_wr(uint8_t dat) override;

	// Inherited via fb_abs_resettable
	virtual void reset() override;

	friend class fb_intcon_mas;
	friend class fb_intcon;


    virtual uint8_t peek(uint32_t addr) override;
    virtual void poke(uint32_t addr, uint8_t dat);

	
protected:

private:
	fb_intcon& intcon;
	fb_abs_master* mas;

	fb_intcon_mas* crossbar_mas;
	uint32_t crossbar_addr;
	bool crossbar_we;
	int crossbar_idx;	//index of pend slave dev no
	bool crossbar_connected;
	bool crossbar_pend;
	bool crossbar_d_wr_pend;
	uint8_t crossbar_d_wr;

	void do_discon();
	void do_try_connect();



};

class fb_intcon_mas : public fb_abs_master, public fb_abs_resettable {
public:
	fb_intcon_mas(fb_intcon& _intcon) :
		intcon(_intcon), sla(0) 
	{
		reset();
	}
	// Inherited via fb_abs_master
	virtual void init(fb_abs_slave & _sla) override;
	virtual void fb_set_ACK(fb_ack ack) override;
	virtual void fb_set_D_rd(uint8_t dat) override;

	// Inherited via fb_abs_resettable
	virtual void reset() override;

	friend class fb_intcon_sla;
	friend class fb_intcon;

protected:
	void do_discon();
private:
	fb_intcon& intcon;
	fb_abs_slave* sla;

	fb_intcon_sla* crossbar_sla;
};

class fb_intcon : public fb_abs_resettable {

public:
	fb_intcon(blitter_top& _top, int _slaves, int _masters) :
		top(_top), masters(_masters), slaves(_slaves)
	{
		sla_a = (fb_intcon_sla**)calloc(slaves, sizeof(fb_intcon_sla *));
		mas_a = (fb_intcon_mas**)calloc(masters, sizeof(fb_intcon_mas *));
		
		for (int i = 0; i < slaves; i++)
			sla_a[i] = new fb_intcon_sla(*this);
		for (int i = 0; i < masters; i++)
			mas_a[i] = new fb_intcon_mas(*this);

	}
	   
	fb_intcon_mas* getMas(int index) { 
		if (index < 0 || index >= masters)
			throw std::out_of_range("out of range");
		else
			return mas_a[index]; 
	};
	fb_intcon_sla* getSla(int index) {
		if (index < 0 || index >= slaves)
			throw std::out_of_range("out of range");
		else
			return sla_a[index];
	}

	friend class fb_intcon_mas;
	friend class fb_intcon_sla;

	virtual void reset() override;


protected:
	blitter_top& top;

	void do_pending(fb_intcon_mas *released_mas);

private:
	int masters, slaves;
	fb_intcon_mas** mas_a;
	fb_intcon_sla** sla_a;



};



#endif // !_FB_INTCON_H_
