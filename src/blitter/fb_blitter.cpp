#include "fb_blitter.h"

#include <iostream>

fb_blitter::~fb_blitter()
{
}

void fb_blitter::reset()
{
	r_cha_A_data 		= 0;
	r_cha_A_data_pre 	= 0;
	r_cha_B_data 		= 0;
	r_cha_B_data_pre 	= 0;
	r_cha_C_data 		= 0;
	r_cha_A_addr 		= 0;
	r_cha_B_addr 		= 0;
	r_cha_C_addr 		= 0;
	r_cha_D_addr 		= 0;
	r_cha_E_addr 		= 0;
	r_BLTCON_act 		= false;
	r_BLTCON_execA 		= false;
	r_BLTCON_execB 		= false;
	r_BLTCON_execC 		= false;
	r_BLTCON_execD 		= false;
	r_BLTCON_execE 		= false;
	r_BLTCON_mode 		= blitter_bppmode::bpp_1;
	r_BLTCON_cell 		= false;
	r_BLTCON_line 		= false;
	r_BLTCON_collision 	= false;
	r_shift_A 		= 0;
	r_shift_B 		= 0;
	r_width 		= 0;
	r_height 		= 0;
	r_FUNCGEN 		= 0;
	r_mask_first 		= 0;
	r_mask_last 		= 0;

}

void fb_blitter::tick(bool sys)
{


/* This was in write process
			if r_blit_state = sFinish then
				r_BLTCON_act <= '0';
			end if;

			if r_blit_state = sLineCalc and fb_syscon_i.cpu_clks(FB_CPUCLKINDEX(G_SPEED)).cpu_clken = '1' then
				-- count down major axis
				v_major_ctr := unsigned(r_width & r_height) - 1;
				r_width <= v_major_ctr(15 downto 8);
				r_height <= v_major_ctr(7 downto 0);

				-- save this as the current pixel's mask
				i_reg_line_pxmaskcur <= r_cha_A_data;

				v_err_acc := signed(r_cha_A_addr(15 downto 0)) - signed(r_cha_A_stride(15 downto 0));
				v_reg_line_minor := '0';
				if (v_err_acc(15) = '1') then
					v_reg_line_minor := '1';
					v_err_acc := v_err_acc + signed(r_cha_B_addr(15 downto 0));
				end if;


				-- rotate the pixel mask if we need to
				if r_BLTCON_mode(0) = '0'	or (v_reg_line_minor = '1' and r_BLTCON_mode(1) = '0') then
					r_cha_A_data <= r_cha_A_data(0) & r_cha_A_data(7 downto 1);	-- roll right
				elsif r_BLTCON_mode = "11" and v_reg_line_minor = '1' then
					r_cha_A_data <= r_cha_A_data(6 downto 0) & r_cha_A_data(7);	-- roll left
				end if;

				r_cha_A_addr(15 downto 0) <= std_logic_vector(v_err_acc);
				i_reg_line_minor <= v_reg_line_minor;

			end if;

			if (r_mas_state = waitack and fb_mas_s2m_i.ack = '1') 
			or ((r_blit_state = sMemAccC_min or r_blit_state = sMemAccD_min) and
				 fb_syscon_i.cpu_clks(FB_CPUCLKINDEX(G_SPEED)).cpu_clken = '1'
				) then
				-- update addresses after a memory access
				case r_blit_state is
					case sMemAccA:
						r_cha_A_addr(15 downto 0) <= i_next_addr_blit;
					case sMemAccB:
						r_cha_B_addr(15 downto 0) <= i_next_addr_blit;
					case sMemAccC | sMemAccC_min:
						r_cha_C_addr(15 downto 0) <= i_next_addr_blit;
					case sMemAccD | sMemAccD_min: 
						r_cha_D_addr(15 downto 0) <= i_next_addr_blit;
					case sMemAccE: 
						r_cha_E_addr(15 downto 0) <= i_next_addr_blit;
					case others: null;
				end case;

				-- update previous/next data after a memory access
				case r_blit_state is
					case sMemAccA:
						r_cha_A_data_pre <= r_cha_A_data(6 downto 0);			
						r_cha_A_data <= fb_mas_s2m_i.D_rd;
					case sMemAccB:
						r_cha_B_data_pre <= r_cha_B_data(6 downto 0);			
						r_cha_B_data <= fb_mas_s2m_i.D_rd;
					case sMemAccC | sMemAccC_min:
						r_cha_C_data <= fb_mas_s2m_i.D_rd;
					case others: null;
				end case;
			end if;
			
			if (r_blit_state = sMemAccD) and (or_reduce(i_cha_D_data) /= '0') then
				r_BLTCON_collision <= '0';
			end if;


*/



}


void fb_blitter::init() {

	reset();
}

void fb_blitter::gotByte(uint8_t channel, int8_t dat)
{

}

void fb_blitter::write_regs(uint8_t addr, uint8_t dat)
{

	switch(addr - A_BLIT_BASE) {
		case A_BLITOFFS_BLTCON:
			if (dat & 0x80) {
				r_BLTCON_act = true;
				r_BLTCON_cell = dat & 0x40;
				r_BLTCON_mode = (blitter_bppmode)((dat >> 4) & 0x03);
				r_BLTCON_line = dat & 0x08;
				r_BLTCON_collision = dat & 0x04;
				r_BLTCON_wrap = dat & 0x02;
			} else {
				r_BLTCON_execA = dat & 0x01;
				r_BLTCON_execB = dat & 0x02;
				r_BLTCON_execC = dat & 0x04;
				r_BLTCON_execD = dat & 0x08;
				r_BLTCON_execE = dat & 0x10;
			}
		case A_BLITOFFS_FUNCGEN:
			r_FUNCGEN = dat;
		case A_BLITOFFS_WIDTH:
			r_width = dat;
		case A_BLITOFFS_HEIGHT:
			r_height = dat;
		case A_BLITOFFS_SHIFT: // TODO: maybe split to two addresses to make calcs easier?
			r_shift_A <= dat & 0x07;
			r_shift_B <= dat & (0x70) >> 4;
		case A_BLITOFFS_MASK_FIRST:
			r_mask_first <= dat;
		case A_BLITOFFS_MASK_LAST:
			r_mask_last <= dat;
		case A_BLITOFFS_DATA_A:
			r_cha_A_data_pre <= r_cha_A_data & 0x7F; //TODO: why?			
			r_cha_A_data <= dat;
		case A_BLITOFFS_ADDR_A + 0:
			ADDR_BANK_SET(r_cha_A_addr, dat);
		case A_BLITOFFS_ADDR_A + 1:
			ADDR_HI_SET(r_cha_A_addr, dat);
		case A_BLITOFFS_ADDR_A + 2:
			ADDR_LO_SET(r_cha_A_addr, dat);
		case A_BLITOFFS_DATA_B:
			r_cha_B_data_pre <= r_cha_B_data & 0x7F; //TODO: why?			
			r_cha_B_data <= dat;
		case A_BLITOFFS_ADDR_B + 0:
			ADDR_BANK_SET(r_cha_B_addr, dat);
		case A_BLITOFFS_ADDR_B + 1:
			ADDR_HI_SET(r_cha_B_addr, dat);
		case A_BLITOFFS_ADDR_B + 2:
			ADDR_LO_SET(r_cha_B_addr, dat);
		case A_BLITOFFS_ADDR_C + 0:
			ADDR_BANK_SET(r_cha_C_addr, dat);
		case A_BLITOFFS_ADDR_C + 1:
			ADDR_HI_SET(r_cha_C_addr, dat);
		case A_BLITOFFS_ADDR_C + 2:
			ADDR_LO_SET(r_cha_C_addr, dat);
		case A_BLITOFFS_ADDR_D + 0:
			ADDR_BANK_SET(r_cha_D_addr, dat);
		case A_BLITOFFS_ADDR_D + 1:
			ADDR_HI_SET(r_cha_D_addr, dat);
		case A_BLITOFFS_ADDR_D + 2:
			ADDR_LO_SET(r_cha_D_addr, dat);
		case A_BLITOFFS_ADDR_E + 0:
			ADDR_BANK_SET(r_cha_E_addr, dat);
		case A_BLITOFFS_ADDR_E + 1:
			ADDR_HI_SET(r_cha_E_addr, dat);
		case A_BLITOFFS_ADDR_E + 2:
			ADDR_LO_SET(r_cha_E_addr, dat);
		case A_BLITOFFS_STRIDE_A:
			r_cha_A_stride = r_cha_A_stride & 0x00FF | ((uint16_t)dat) << 8; // note 16 bits for line mode
		case A_BLITOFFS_STRIDE_A + 1:
			r_cha_A_stride = r_cha_A_stride & 0xFF00 | ((uint16_t)dat) << 0;
		case A_BLITOFFS_STRIDE_B:
			STRIDE_HI_SET(r_cha_B_stride, dat);
		case A_BLITOFFS_STRIDE_B + 1:
			STRIDE_LO_SET(r_cha_B_stride, dat);
		case A_BLITOFFS_STRIDE_C:
			STRIDE_HI_SET(r_cha_C_stride, dat);
		case A_BLITOFFS_STRIDE_C + 1:
			STRIDE_LO_SET(r_cha_B_stride, dat);
		case A_BLITOFFS_STRIDE_D:
			STRIDE_HI_SET(r_cha_D_stride, dat);
		case A_BLITOFFS_STRIDE_D + 1:
			STRIDE_LO_SET(r_cha_B_stride, dat);

		case A_BLITOFFS_ADDR_D_MIN: 
			ADDR_BANK_SET(r_cha_D_addr_min, dat);
		case A_BLITOFFS_ADDR_D_MIN+1: 
			ADDR_HI_SET(r_cha_D_addr_min, dat);
		case A_BLITOFFS_ADDR_D_MIN+2: 
			ADDR_LO_SET(r_cha_D_addr_min, dat);

		case A_BLITOFFS_ADDR_D_MAX: 
			ADDR_BANK_SET(r_cha_D_addr_max, dat);
		case A_BLITOFFS_ADDR_D_MAX+1: 
			ADDR_HI_SET(r_cha_D_addr_max, dat);
		case A_BLITOFFS_ADDR_D_MAX+2: 
			ADDR_LO_SET(r_cha_D_addr_max, dat);

	}
}

uint8_t fb_blitter::read_regs(uint8_t addr) {
	switch (addr - A_BLIT_BASE) {
		case A_BLITOFFS_BLTCON :
			return 	(r_BLTCON_act?0x80:0x00) +
				(r_BLTCON_cell?0x40:0x00) +
				(((uint8_t)r_BLTCON_mode) << 4) +
				(r_BLTCON_line?0x08:0x00) +
				(r_BLTCON_collision?0x04:0x00) +
				(r_BLTCON_wrap?0x02:0x00);
		case A_BLITOFFS_FUNCGEN :
			return r_FUNCGEN;
		case A_BLITOFFS_WIDTH :
			return r_width;
		case A_BLITOFFS_HEIGHT :			
			return r_height;
		case A_BLITOFFS_SHIFT :
			return (r_shift_B << 4) | r_shift_A;
		case A_BLITOFFS_MASK_FIRST :
			return r_mask_first;
		case A_BLITOFFS_MASK_LAST :
			return r_mask_last;
		case A_BLITOFFS_DATA_A :
			return r_cha_A_data;
		case A_BLITOFFS_ADDR_A :				
			return BANK8(r_cha_A_addr);
		case A_BLITOFFS_ADDR_A + 1 :
			return HI8(r_cha_A_addr);
		case A_BLITOFFS_ADDR_A + 2 :
			return LO8(r_cha_A_addr);
		case A_BLITOFFS_DATA_B :
			return r_cha_B_data;
		case A_BLITOFFS_ADDR_B :
			return BANK8(r_cha_B_addr);
		case A_BLITOFFS_ADDR_B + 1 :
			return HI8(r_cha_B_addr);
		case A_BLITOFFS_ADDR_B + 2 :
			return LO8(r_cha_B_addr);
		case A_BLITOFFS_ADDR_C :
			return BANK8(r_cha_C_addr);
		case A_BLITOFFS_ADDR_C + 1 :
			return HI8(r_cha_C_addr);
		case A_BLITOFFS_ADDR_C + 2 :
			return LO8(r_cha_C_addr);
		case A_BLITOFFS_ADDR_D :
			return BANK8(r_cha_D_addr);
		case A_BLITOFFS_ADDR_D + 1 :
			return HI8(r_cha_D_addr);
		case A_BLITOFFS_ADDR_D + 2 :
			return LO8(r_cha_D_addr);
		case A_BLITOFFS_ADDR_E :
			return BANK8(r_cha_E_addr);
		case A_BLITOFFS_ADDR_E + 1 :
			return HI8(r_cha_E_addr);
		case A_BLITOFFS_ADDR_E + 2 :
			return LO8(r_cha_E_addr);
		case A_BLITOFFS_STRIDE_A :
			return HI8(r_cha_A_stride);
		case A_BLITOFFS_STRIDE_A + 1 :
			return LO8(r_cha_A_stride);
		case A_BLITOFFS_STRIDE_B :
			return STRIDE_HI_GET(r_cha_B_stride);
		case A_BLITOFFS_STRIDE_B + 1 :
			return LO8(r_cha_B_stride);
		case A_BLITOFFS_STRIDE_C :
			return STRIDE_HI_GET(r_cha_C_stride);
		case A_BLITOFFS_STRIDE_C + 1 :
			return LO8(r_cha_C_stride);
		case A_BLITOFFS_STRIDE_D :
			return STRIDE_HI_GET(r_cha_D_stride);
		case A_BLITOFFS_STRIDE_D + 1 :
			return LO8(r_cha_D_stride);		

		case A_BLITOFFS_ADDR_D_MIN :
			return BANK8(r_cha_D_addr_min);
		case A_BLITOFFS_ADDR_D_MIN + 1 :
			return HI8(r_cha_D_addr_min);
		case A_BLITOFFS_ADDR_D_MIN + 2 :
			return LO8(r_cha_D_addr_min);

		case A_BLITOFFS_ADDR_D_MAX :
			return BANK8(r_cha_D_addr_max);
		case A_BLITOFFS_ADDR_D_MAX + 1 :
			return HI8(r_cha_D_addr_max);
		case A_BLITOFFS_ADDR_D_MAX + 2 :
			return LO8(r_cha_D_addr_max);

		default:
			return 0xFF;
	}
}

void fb_blitter_sla::init(fb_abs_master & mas)
{
	this->mas = &mas;
}

void fb_blitter_sla::fb_set_cyc(fb_cyc_action cyc)
{
	if (cyc == stop) {
		state = idle;
	}
	else {
		if (we)
			state = actwaitwr;
		else if (mas)
		{
			mas->fb_set_D_rd(blitter.read_regs(addr));
			mas->fb_set_ACK(ack);
		}
	}

}

void fb_blitter_sla::fb_set_A(uint32_t addr, bool we)
{
	this->addr = addr;
	this->we = we;

}

void fb_blitter_sla::fb_set_D_wr(uint8_t dat)
{
	if (state == actwaitwr) {
		blitter.write_regs(addr, dat);
		mas->fb_set_ACK(ack);
		state = idle;
	}
}

void fb_blitter_sla::reset()
{
	state = idle;
}

void fb_blitter_mas::init(fb_abs_slave & sla)
{
	this->sla = &sla;
	reset();
}

void fb_blitter_mas::fb_set_ACK(fb_ack ack)
{
	state = idle;
	if (sla)
		sla->fb_set_cyc(stop);
}

void fb_blitter_mas::fb_set_D_rd(uint8_t dat)
{
	if (state == act) {
		blitter.gotByte(cur_chan, dat);
	}
}

void fb_blitter_mas::reset()
{
	state = idle;
}

bool fb_blitter_mas::getByte(uint32_t addr, uint8_t channel)
{
	if (state == idle) {
		if (sla) {
			cur_addr = addr;
			cur_chan = channel;
			state = act;
			sla->fb_set_A(addr, 0);
			sla->fb_set_cyc(start);
		}
		else {
			blitter.gotByte(channel, 0);
		}
		return true;
	}

	return false;
}

inline uint16_t fb_blitter::blit_addr_next(
		blit_addr_direction 	direction,
		bool			mode_cell,
		uint16_t		bytes_stride,
		uint8_t			width,
		bool			wrap,
		uint16_t		addr_in,
		uint16_t		addr_min,
		uint16_t		addr_max
	) {
		uint16_t i_addr_next;

	if (mode_cell) {
		switch (direction) {
			case blit_addr_direction::cha_e: 
				i_addr_next = addr_in + 1;
			case blit_addr_direction::spr_wrap: 
				if ((addr_in & 0x07) == 0x07)
					i_addr_next = addr_in + bytes_stride - (((uint16_t)width) << 3) - 7;
				else
					i_addr_next = addr_in - (((uint16_t)width) << 3) + 1;
			case blit_addr_direction::plot_up: 
				if ((addr_in & 0x07) == 0x00)
					i_addr_next = addr_in - bytes_stride + 7;
				else
					i_addr_next = addr_in - 1;
			case blit_addr_direction::plot_down: 
				if ((addr_in & 0x07) == 0x07)
					i_addr_next = addr_in + bytes_stride - 7;
				else
					i_addr_next = addr_in + 1;
			case blit_addr_direction::plot_left: 
				i_addr_next = addr_in - 8;
			case blit_addr_direction::plot_right: 
				i_addr_next = addr_in + 8;
			default: 
				i_addr_next = addr_in;
		}
	} else {
		switch (direction) {
			case blit_addr_direction::cha_e:  
				i_addr_next <= addr_in + 1;
			case blit_addr_direction::spr_wrap: 
				i_addr_next <= addr_in + bytes_stride - ((uint16_t)width);
			case blit_addr_direction::plot_up: 
				i_addr_next <= addr_in - bytes_stride;
			case blit_addr_direction::plot_down: 
				i_addr_next <= addr_in + bytes_stride;
			case blit_addr_direction::plot_left: 
				i_addr_next <= addr_in - 1;
			case blit_addr_direction::plot_right: 
				i_addr_next <= addr_in + 1;
			default: 
				i_addr_next <= addr_in;
		};
	};

	if (wrap) {
		if (i_addr_next > addr_max)
			i_addr_next = i_addr_next - addr_max + addr_min;
		else if (i_addr_next < addr_min)
			i_addr_next = i_addr_next - addr_min + addr_max;
	}

	return i_addr_next;
}