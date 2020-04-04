#include "fb_blitter.h"

#include <iostream>
#include "blitter_top.h"


//TODO: check C_data_min - should it read on min access!? - if not, sort out fpga

fb_blitter::~fb_blitter()
{
}

void fb_blitter::reset()
{
    r_cha_A_data = 0;
    r_cha_A_data_pre = 0;
    r_cha_B_data = 0;
    r_cha_B_data_pre = 0;
    r_cha_C_data = 0;
    r_cha_A_addr = 0;
    r_cha_B_addr = 0;
    r_cha_C_addr = 0;
    r_cha_D_addr = 0;
    r_cha_E_addr = 0;
    r_BLTCON_act = false;
    r_BLTCON_execA = false;
    r_BLTCON_execB = false;
    r_BLTCON_execC = false;
    r_BLTCON_execD = false;
    r_BLTCON_execE = false;
    r_BLTCON_mode = blitter_bppmode::bpp_1;
    r_BLTCON_cell = false;
    r_BLTCON_line = false;
    r_BLTCON_collision = false;
    r_shift_A = 0;
    r_shift_B = 0;
    r_width = 0;
    r_height = 0;
    r_FUNCGEN = 0;
    r_mask_first = 0;
    r_mask_last = 0;
    r_cha_D_addr_min = 0;
    r_cha_D_addr_max = 0;
    r_cha_A_stride = 0;
    r_cha_B_stride = 0;
    r_cha_C_stride = 0;
    r_cha_D_stride = 0;


    r_blit_state = state_type::sIdle;
    r_clken_addr_calc_start = false;
    r_data_ready = false;

    //p_ctl
    r_row_countdn = 0;
    r_y_count = 0;
    top.setHALT(compno_BLIT, false);
    r_cha_A_last_mask = false;

}

void fb_blitter::tick(bool sys)
{


    bool next_r_clken_addr_calc_start = false;
    state_cha_A next_r_accA_state_cur = r_accA_state_cur;
    state_type next_r_blit_state = r_blit_state;
    uint8_t next_r_row_countdn = r_row_countdn;

    // p_blit_state
    // slightly changed as we don't wait for ack's on writes

    if (
        !(r_blit_state == state_type::sMemAccA ||
            r_blit_state == state_type::sMemAccB ||
            r_blit_state == state_type::sMemAccC ||
            r_blit_state == state_type::sMemAccD ||
            r_blit_state == state_type::sMemAccE
            ) || r_data_ready
        ) {
        next_r_accA_state_cur = get_accA_state_next();
        next_r_blit_state = get_state_next();
        next_r_clken_addr_calc_start = true;

        //p_ctl
        if (r_blit_state == state_type::sStart) {
            next_r_row_countdn = r_width;
            r_y_count = r_height;
            r_cha_A_first = true;
            top.setHALT(compno_BLIT, true);
            r_cha_A_last_mask = false;
            std::cerr << "blit:started\n";
            //} else if (fb_mas_s2m_i.ack = '1' or r_BLTCON_execD = '0') and r_blit_state = sMemAccD then
        }
        else if (r_blit_state == state_type::sMemAccD) {
            if (r_row_countdn == 0) {
                next_r_row_countdn = r_width;
                r_cha_A_first = true;
                r_y_count = r_y_count - 1;
            }
            else {
                next_r_row_countdn = r_row_countdn - 1;
                //-- TODO: CHECK THIS thoroughly - seems to work!
                //--					if i_state_next = sMemAccA then
                r_cha_A_first = false;
                //--					end if;
            }
        }
        else if (r_data_ready && r_blit_state == state_type::sMemAccA) {
            r_cha_A_last_mask = get_cha_A_last();
        }
        else if (r_blit_state == state_type::sFinish) {
            top.setHALT(compno_BLIT, false);
            std::cerr << "blit:finished\n";
        }
    }



    //p_regs_wr
    if (r_blit_state == state_type::sFinish)
        r_BLTCON_act = false;

    static uint16_t v_major_ctr;
    static bool v_reg_line_minor;
    static int16_t v_err_acc;


    if (r_blit_state == state_type::sLineCalc) {
        // count down major axis
        v_major_ctr = ((((uint16_t)r_width) << 8) | r_height) - 1;
        r_width = v_major_ctr >> 8;
        r_height = (uint8_t)v_major_ctr;

        // save this as the current pixel's mask
        r_line_pxmaskcur = r_cha_A_data;

        v_err_acc = ((int16_t)r_cha_A_addr) - ((int16_t)r_cha_A_stride);
        v_reg_line_minor = false;
        if (v_err_acc > 0) {
            v_reg_line_minor = true;
            v_err_acc = v_err_acc + ((int16_t)r_cha_B_addr);
        }

        // rotate the pixel mask if we need to
        if (!((int)r_BLTCON_mode & 0x01)
            || (v_reg_line_minor && ((int)r_BLTCON_mode & 0x02))
            ) {
            //roll right
            r_cha_A_data = ((r_cha_A_data & 0x01) << 7) | (r_cha_A_data >> 1);
        }
        else if (((int)r_BLTCON_mode == 3) && v_reg_line_minor) {
            // roll left
            r_cha_A_data = (r_cha_A_data << 1) | (r_cha_A_data & 0x80) >> 7;
        }

        r_cha_A_addr = (r_cha_A_addr & 0xFF0000) | (uint32_t)v_err_acc;
        r_line_minor = v_reg_line_minor;

    }





    if (
        (r_data_ready && (
            r_blit_state == state_type::sMemAccA ||
            r_blit_state == state_type::sMemAccB ||
            r_blit_state == state_type::sMemAccC ||
            r_blit_state == state_type::sMemAccD ||
            r_blit_state == state_type::sMemAccE
            )
            ) ||
        r_blit_state == state_type::sMemAccC_min ||
        r_blit_state == state_type::sMemAccD_min
        ) {

        // update addresses after a memory access
        switch (r_blit_state) {
        case state_type::sMemAccA:
            ADDR_16_SET(r_cha_A_addr, get_next_addr_blit());
            std::cerr << "addr_A <= " << std::hex << (int)r_cha_A_addr << "\n";
            break;
        case state_type::sMemAccB:
            ADDR_16_SET(r_cha_B_addr, get_next_addr_blit());
            std::cerr << "addr_B <= " << std::hex << (int)r_cha_B_addr << "\n";
            break;
        case state_type::sMemAccC:
        case state_type::sMemAccC_min:
            ADDR_16_SET(r_cha_C_addr, get_next_addr_blit());
            std::cerr << "addr_C <= " << std::hex << (int)r_cha_C_addr << "\n";
            break;
        case state_type::sMemAccD:
        case state_type::sMemAccD_min:
            ADDR_16_SET(r_cha_D_addr, get_next_addr_blit());
            std::cerr << "addr_D <= " << std::hex << (int)r_cha_D_addr << "\n";
            break;
        case state_type::sMemAccE:
            ADDR_16_SET(r_cha_E_addr, get_next_addr_blit());
            std::cerr << "addr_E <= " << std::hex << (int)r_cha_E_addr << "\n";
            break;
        }

        // update previous/next data after a memory access
        switch (r_blit_state) {
        case state_type::sMemAccA:
            r_cha_A_data_pre = r_cha_A_data & 0x7F;
            r_cha_A_data = r_data_read;
            break;
        case state_type::sMemAccB:
            r_cha_B_data_pre = r_cha_B_data & 0x7F;
            r_cha_B_data = r_data_read;
            break;
        case state_type::sMemAccC:
            r_cha_C_data = r_data_read;
            break;
        }
    }


    switch (next_r_blit_state)
    {
    case state_type::sMemAccA:
        if (!mas.getByte(r_cha_A_addr)) {
            next_r_blit_state = r_blit_state;
        }
        break;
    case state_type::sMemAccB:
        if (!mas.getByte(r_cha_B_addr)) {
            next_r_blit_state = r_blit_state;
        }
        break;
    case state_type::sMemAccC:
        if (!mas.getByte(r_cha_C_addr)) {
            next_r_blit_state = r_blit_state;
        }
        break;
    case state_type::sMemAccD:
    {
        uint8_t d = get_funcgen_data();
        if (r_BLTCON_execD) {
            if (!mas.setByte(r_cha_D_addr, d)) {
                next_r_blit_state = r_blit_state;
            }
        }
        else {
            r_data_ready = true;
        }
        if (d != 0)
            r_BLTCON_collision = false;
    }
    break;
    case state_type::sMemAccE:
        if (!mas.setByte(r_cha_E_addr, r_cha_C_data)) {
            next_r_blit_state = r_blit_state;
        }
        break;
    }


    r_blit_state = next_r_blit_state;
    r_row_countdn = next_r_row_countdn;
    r_clken_addr_calc_start = next_r_clken_addr_calc_start;
    r_accA_state_cur = next_r_accA_state_cur;

}


void fb_blitter::init() {

    reset();
}

void fb_blitter::gotByte(int8_t dat)
{
    r_data_read = dat;
    r_data_ready = true;

}

void fb_blitter::write_regs(uint8_t addr, uint8_t dat)
{

    std::cerr << "BLT:" << std::hex << (int)addr << ":" << (int)dat << "\n";


    switch (addr - A_BLIT_BASE) {
    case A_BLITOFFS_BLTCON:
        std::cerr << "BLTCON:" << std::hex << (int)dat << "\n";
        if (dat & 0x80) {
            r_BLTCON_act = true;
            r_BLTCON_cell = dat & 0x40;
            r_BLTCON_mode = (blitter_bppmode)((dat >> 4) & 0x03);
            r_BLTCON_line = dat & 0x08;
            r_BLTCON_collision = dat & 0x04;
            r_BLTCON_wrap = dat & 0x02;
        }
        else {
            r_BLTCON_execA = dat & 0x01;
            r_BLTCON_execB = dat & 0x02;
            r_BLTCON_execC = dat & 0x04;
            r_BLTCON_execD = dat & 0x08;
            r_BLTCON_execE = dat & 0x10;
        }
        break;
    case A_BLITOFFS_FUNCGEN:
        r_FUNCGEN = dat;
        break;
    case A_BLITOFFS_WIDTH:
        r_width = dat;
        break;
    case A_BLITOFFS_HEIGHT:
        r_height = dat;
        break;
    case A_BLITOFFS_SHIFT: // TODO: maybe split to two addresses to make calcs easier?
        r_shift_A = dat & 0x07;
        r_shift_B = dat & (0x70) >> 4;
        break;
    case A_BLITOFFS_MASK_FIRST:
        r_mask_first = dat;
        break;
    case A_BLITOFFS_MASK_LAST:
        r_mask_last = dat;
        break;
    case A_BLITOFFS_DATA_A:
        r_cha_A_data_pre = r_cha_A_data & 0x7F; //TODO: why?			
        r_cha_A_data = dat;
        break;
    case A_BLITOFFS_ADDR_A + 0:
        ADDR_BANK_SET(r_cha_A_addr, dat);
        break;
    case A_BLITOFFS_ADDR_A + 1:
        ADDR_HI_SET(r_cha_A_addr, dat);
        break;
    case A_BLITOFFS_ADDR_A + 2:
        ADDR_LO_SET(r_cha_A_addr, dat);
        break;
    case A_BLITOFFS_DATA_B:
        r_cha_B_data_pre = r_cha_B_data & 0x7F; //TODO: why?			
        r_cha_B_data = dat;
        break;
    case A_BLITOFFS_ADDR_B + 0:
        ADDR_BANK_SET(r_cha_B_addr, dat);
        break;
    case A_BLITOFFS_ADDR_B + 1:
        ADDR_HI_SET(r_cha_B_addr, dat);
        break;
    case A_BLITOFFS_ADDR_B + 2:
        ADDR_LO_SET(r_cha_B_addr, dat);
        break;
    case A_BLITOFFS_ADDR_C + 0:
        ADDR_BANK_SET(r_cha_C_addr, dat);
        break;
    case A_BLITOFFS_ADDR_C + 1:
        ADDR_HI_SET(r_cha_C_addr, dat);
        break;
    case A_BLITOFFS_ADDR_C + 2:
        ADDR_LO_SET(r_cha_C_addr, dat);
        break;
    case A_BLITOFFS_ADDR_D + 0:
        ADDR_BANK_SET(r_cha_D_addr, dat);
        break;
    case A_BLITOFFS_ADDR_D + 1:
        ADDR_HI_SET(r_cha_D_addr, dat);
        break;
    case A_BLITOFFS_ADDR_D + 2:
        ADDR_LO_SET(r_cha_D_addr, dat);
        break;
    case A_BLITOFFS_ADDR_E + 0:
        ADDR_BANK_SET(r_cha_E_addr, dat);
        break;
    case A_BLITOFFS_ADDR_E + 1:
        ADDR_HI_SET(r_cha_E_addr, dat);
        break;
    case A_BLITOFFS_ADDR_E + 2:
        ADDR_LO_SET(r_cha_E_addr, dat);
        break;
    case A_BLITOFFS_STRIDE_A:
        r_cha_A_stride = r_cha_A_stride & 0x00FF | ((uint16_t)dat) << 8; // note 16 bits for line mode
        break;
    case A_BLITOFFS_STRIDE_A + 1:
        r_cha_A_stride = r_cha_A_stride & 0xFF00 | ((uint16_t)dat) << 0;
        break;
    case A_BLITOFFS_STRIDE_B:
        STRIDE_HI_SET(r_cha_B_stride, dat);
        break;
    case A_BLITOFFS_STRIDE_B + 1:
        STRIDE_LO_SET(r_cha_B_stride, dat);
        break;
    case A_BLITOFFS_STRIDE_C:
        STRIDE_HI_SET(r_cha_C_stride, dat);
        break;
    case A_BLITOFFS_STRIDE_C + 1:
        STRIDE_LO_SET(r_cha_C_stride, dat);
        break;
    case A_BLITOFFS_STRIDE_D:
        STRIDE_HI_SET(r_cha_D_stride, dat);
        break;
    case A_BLITOFFS_STRIDE_D + 1:
        STRIDE_LO_SET(r_cha_D_stride, dat);
        break;
    case A_BLITOFFS_ADDR_D_MIN:
        ADDR_BANK_SET(r_cha_D_addr_min, dat);
        break;
    case A_BLITOFFS_ADDR_D_MIN + 1:
        ADDR_HI_SET(r_cha_D_addr_min, dat);
        break;
    case A_BLITOFFS_ADDR_D_MIN + 2:
        ADDR_LO_SET(r_cha_D_addr_min, dat);
        break;
    case A_BLITOFFS_ADDR_D_MAX:
        ADDR_BANK_SET(r_cha_D_addr_max, dat);
        break;
    case A_BLITOFFS_ADDR_D_MAX + 1:
        ADDR_HI_SET(r_cha_D_addr_max, dat);
        break;
    case A_BLITOFFS_ADDR_D_MAX + 2:
        ADDR_LO_SET(r_cha_D_addr_max, dat);
        break;
    }
}

uint8_t fb_blitter::read_regs(uint8_t addr) {
    switch (addr - A_BLIT_BASE) {
    case A_BLITOFFS_BLTCON:
        return 	(r_BLTCON_act ? 0x80 : 0x00) +
            (r_BLTCON_cell ? 0x40 : 0x00) +
            (((uint8_t)r_BLTCON_mode) << 4) +
            (r_BLTCON_line ? 0x08 : 0x00) +
            (r_BLTCON_collision ? 0x04 : 0x00) +
            (r_BLTCON_wrap ? 0x02 : 0x00);
    case A_BLITOFFS_FUNCGEN:
        return r_FUNCGEN;
    case A_BLITOFFS_WIDTH:
        return r_width;
    case A_BLITOFFS_HEIGHT:
        return r_height;
    case A_BLITOFFS_SHIFT:
        return (r_shift_B << 4) | r_shift_A;
    case A_BLITOFFS_MASK_FIRST:
        return r_mask_first;
    case A_BLITOFFS_MASK_LAST:
        return r_mask_last;
    case A_BLITOFFS_DATA_A:
        return r_cha_A_data;
    case A_BLITOFFS_ADDR_A:
        return BANK8(r_cha_A_addr);
    case A_BLITOFFS_ADDR_A + 1:
        return HI8(r_cha_A_addr);
    case A_BLITOFFS_ADDR_A + 2:
        return LO8(r_cha_A_addr);
    case A_BLITOFFS_DATA_B:
        return r_cha_B_data;
    case A_BLITOFFS_ADDR_B:
        return BANK8(r_cha_B_addr);
    case A_BLITOFFS_ADDR_B + 1:
        return HI8(r_cha_B_addr);
    case A_BLITOFFS_ADDR_B + 2:
        return LO8(r_cha_B_addr);
    case A_BLITOFFS_ADDR_C:
        return BANK8(r_cha_C_addr);
    case A_BLITOFFS_ADDR_C + 1:
        return HI8(r_cha_C_addr);
    case A_BLITOFFS_ADDR_C + 2:
        return LO8(r_cha_C_addr);
    case A_BLITOFFS_ADDR_D:
        return BANK8(r_cha_D_addr);
    case A_BLITOFFS_ADDR_D + 1:
        return HI8(r_cha_D_addr);
    case A_BLITOFFS_ADDR_D + 2:
        return LO8(r_cha_D_addr);
    case A_BLITOFFS_ADDR_E:
        return BANK8(r_cha_E_addr);
    case A_BLITOFFS_ADDR_E + 1:
        return HI8(r_cha_E_addr);
    case A_BLITOFFS_ADDR_E + 2:
        return LO8(r_cha_E_addr);
    case A_BLITOFFS_STRIDE_A:
        return HI8(r_cha_A_stride);
    case A_BLITOFFS_STRIDE_A + 1:
        return LO8(r_cha_A_stride);
    case A_BLITOFFS_STRIDE_B:
        return STRIDE_HI_GET(r_cha_B_stride);
    case A_BLITOFFS_STRIDE_B + 1:
        return LO8(r_cha_B_stride);
    case A_BLITOFFS_STRIDE_C:
        return STRIDE_HI_GET(r_cha_C_stride);
    case A_BLITOFFS_STRIDE_C + 1:
        return LO8(r_cha_C_stride);
    case A_BLITOFFS_STRIDE_D:
        return STRIDE_HI_GET(r_cha_D_stride);
    case A_BLITOFFS_STRIDE_D + 1:
        return LO8(r_cha_D_stride);

    case A_BLITOFFS_ADDR_D_MIN:
        return BANK8(r_cha_D_addr_min);
    case A_BLITOFFS_ADDR_D_MIN + 1:
        return HI8(r_cha_D_addr_min);
    case A_BLITOFFS_ADDR_D_MIN + 2:
        return LO8(r_cha_D_addr_min);

    case A_BLITOFFS_ADDR_D_MAX:
        return BANK8(r_cha_D_addr_max);
    case A_BLITOFFS_ADDR_D_MAX + 1:
        return HI8(r_cha_D_addr_max);
    case A_BLITOFFS_ADDR_D_MAX + 2:
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
    std::cerr << "fb_blitter_mas:ACK\n";
    state = idle;
    if (sla)
        sla->fb_set_cyc(stop);
}

void fb_blitter_mas::fb_set_D_rd(uint8_t dat)
{
    if (state == act) {
        std::cerr << "fb_blitter_mas:GOTBYTE\n";
        blitter.gotByte(dat);
    }
}

void fb_blitter_mas::reset()
{
    state = idle;
}

bool fb_blitter_mas::getByte(uint32_t addr)
{
    blitter.r_data_ready = false;
    if (state == idle) {
        std::cerr << "fb_blitter_mas:getByte:" << std::hex << (int)addr << "\n";
        if (sla) {
            cur_addr = addr;
            state = act;
            sla->fb_set_A(addr, 0);
            sla->fb_set_cyc(start);
        }
        else {
            blitter.gotByte(0);
        }
        return true;
    }

    return false;
}

bool fb_blitter_mas::setByte(uint32_t addr, uint8_t dat) {
    blitter.r_data_ready = false;
    if (state == idle) {
        std::cerr << "fb_blitter_mas:setByte:" << std::hex << (int)addr << ":" << (int)dat << "\n";
        blitter.r_data_ready = false;
        if (sla) {
            cur_addr = addr;
            state = act_wr;
            sla->fb_set_A(addr, true);
            sla->fb_set_cyc(start);
            sla->fb_set_D_wr(dat);
            blitter.r_data_ready = true;
        }
        return true;
    }
    return false;
}

//p_addr_mux
inline uint16_t fb_blitter::get_next_addr_blit() {
    switch (r_blit_state) {
    case state_type::sMemAccA:
        return blit_addr_next(
            fnDirection(get_cha_A_last(), false),
            false,
            r_cha_A_stride,
            get_chaA_width_in_bytes(),
            false,
            r_cha_A_addr,
            0,
            0
        );
    case state_type::sMemAccB:
        return blit_addr_next(
            fnDirection(get_main_last(), false),
            false,
            r_cha_B_stride,
            r_width,
            false,
            r_cha_B_addr,
            0,
            0
        );
    case state_type::sMemAccC:
    case state_type::sMemAccC_min:
        return blit_addr_next(
            fnDirection(get_main_last(), r_blit_state == state_type::sMemAccC),
            r_BLTCON_cell,
            r_cha_C_stride,
            r_width,
            r_BLTCON_wrap,
            r_cha_C_addr,
            r_cha_D_addr_min,
            r_cha_D_addr_max
        );
    case state_type::sMemAccD:
    case state_type::sMemAccD_min:
        return blit_addr_next(
            fnDirection(get_main_last(), r_blit_state == state_type::sMemAccD),
            r_BLTCON_cell,
            r_cha_D_stride,
            r_width,
            r_BLTCON_wrap,
            r_cha_D_addr,
            r_cha_D_addr_min,
            r_cha_D_addr_max
        );
    case state_type::sMemAccE:
        return blit_addr_next(
            blit_addr_direction::cha_e,
            false,
            0,
            r_width,
            false,
            r_cha_E_addr,
            0,
            0
        );
    default:
        return 0;
    }
}

const char *fb_blitter::dirstr(blit_addr_direction a) {
    switch (a) {
    case blit_addr_direction::cha_e: return "cha_e";
    case blit_addr_direction::plot_up: return "plot_up";
    case blit_addr_direction::plot_down: return "plot_down";
    case blit_addr_direction::plot_right: return "plot_right";
    case blit_addr_direction::plot_left: return "plot_left";
    case blit_addr_direction::spr_wrap: return "spr_wrap";
    case blit_addr_direction::none: return "none";
    default: return "fik";
    }
}

//work.blit_addr component
inline uint16_t fb_blitter::blit_addr_next(
    blit_addr_direction 	direction,
    bool			mode_cell,
    int16_t			bytes_stride,
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
            break;
        case blit_addr_direction::spr_wrap:
            if ((addr_in & 0x07) == 0x07)
                i_addr_next = addr_in + bytes_stride - (((uint16_t)width) << 3) - 7;
            else
                i_addr_next = addr_in - (((uint16_t)width) << 3) + 1;
            break;
        case blit_addr_direction::plot_up:
            if ((addr_in & 0x07) == 0x00)
                i_addr_next = addr_in - bytes_stride + 7;
            else
                i_addr_next = addr_in - 1;
            break;
        case blit_addr_direction::plot_down:
            if ((addr_in & 0x07) == 0x07)
                i_addr_next = addr_in + bytes_stride - 7;
            else
                i_addr_next = addr_in + 1;
            break;
        case blit_addr_direction::plot_left:
            i_addr_next = addr_in - 8;
            break;
        case blit_addr_direction::plot_right:
            i_addr_next = addr_in + 8;
            break;
        default:
            i_addr_next = addr_in;
            break;
        }
    }
    else {
        switch (direction) {
        case blit_addr_direction::cha_e:
            i_addr_next = addr_in + 1;
            break;
        case blit_addr_direction::spr_wrap:
            i_addr_next = addr_in + bytes_stride - ((uint16_t)width);
            break;
        case blit_addr_direction::plot_up:
            i_addr_next = addr_in - bytes_stride;
            break;
        case blit_addr_direction::plot_down:
            i_addr_next = addr_in + bytes_stride;
            break;
        case blit_addr_direction::plot_left:
            i_addr_next = addr_in - 1;
            break;
        case blit_addr_direction::plot_right:
            i_addr_next = addr_in + 1;
            break;
        default:
            i_addr_next = addr_in;
            break;
        };
    };

    if (wrap) {
        if (i_addr_next > addr_max)
            i_addr_next = i_addr_next - addr_max + addr_min;
        else if (i_addr_next < addr_min)
            i_addr_next = i_addr_next - addr_min + addr_max;
    }

    std::cerr << "nxt:" << std::hex
        << (int)addr_in
        << "dir (" << dirstr(direction) << "),"
        << "cell (" << mode_cell << "),"
        << " => "
        << (int)i_addr_next
        << "\n";


    return i_addr_next;
}

inline fb_blitter::blit_addr_direction fb_blitter::fnDirection(bool spr_last, bool line_major) {
    if (r_BLTCON_line) {
        blit_addr_direction v_ret = blit_addr_direction::none;

        if (line_major) {
            if (((int)r_BLTCON_mode) & 0x01) {
                v_ret = blit_addr_direction::plot_up;
            }
            else {
                v_ret = blit_addr_direction::plot_right;
            }
        }
        else {
            switch ((int)r_BLTCON_mode) {
            case 0x0:
                v_ret = blit_addr_direction::plot_down;
                break;
            case 0x01:
                v_ret = blit_addr_direction::plot_right;
                break;
            case 0x02:
                v_ret = blit_addr_direction::plot_up;
                break;
            case 0x03:
                v_ret = blit_addr_direction::plot_left;
                break;
            default:
                v_ret = blit_addr_direction::none;
            }
        }

        if ((v_ret == blit_addr_direction::plot_right) && (r_cha_A_data & 0x80 == 0))
            v_ret = blit_addr_direction::none;

        if ((v_ret == blit_addr_direction::plot_left) && (r_cha_A_data & 0x01) == 0)
            v_ret = blit_addr_direction::none;

        return v_ret;
    }
    else {
        if (spr_last)
            return blit_addr_direction::spr_wrap;
        else
            return blit_addr_direction::plot_right;
    }

}

inline uint8_t fb_blitter::get_chaA_shifted1() {
    switch (r_shift_A) {
    case 1:
        return 		(r_cha_A_data_pre << 7)
            | (r_cha_A_data >> 1);
    case 2:
        return 		(r_cha_A_data_pre << 6)
            | (r_cha_A_data >> 2);
    case 3:
        return 		(r_cha_A_data_pre << 5)
            | (r_cha_A_data >> 3);
    case 4:
        return 		(r_cha_A_data_pre << 4)
            | (r_cha_A_data >> 4);
    case 5:
        return 		(r_cha_A_data_pre << 3)
            | (r_cha_A_data >> 5);
    case 6:
        return 		(r_cha_A_data_pre << 2)
            | (r_cha_A_data >> 6);
    case 7:
        return 		(r_cha_A_data_pre << 1)
            | (r_cha_A_data >> 7);
    default:
        return r_cha_A_data;

    }
}

inline uint8_t fb_blitter::get_chaA_masked() {
    uint8_t sh1 = get_chaA_shifted1();
    if (r_cha_A_last_mask && r_cha_A_first)
        return sh1 & r_mask_last & r_mask_first;
    else if (r_cha_A_last_mask)
        return sh1 & r_mask_last;
    else if (r_cha_A_first)
        return sh1 & r_mask_first;
    else
        return sh1;
}

//note this is clocked in fpga
inline uint8_t fb_blitter::get_chaA_shifted2() {

    uint8_t i_cha_A_data_masked = get_chaA_masked();

    switch (r_accA_state_cur) {
    case state_cha_A::sShiftA1:
        return i_cha_A_data_masked << 1;
    case state_cha_A::sShiftA2:
        return i_cha_A_data_masked << 2;
    case state_cha_A::sShiftA3:
        return i_cha_A_data_masked << 3;
    case state_cha_A::sShiftA4:
        return i_cha_A_data_masked << 4;
    case state_cha_A::sShiftA5:
        return i_cha_A_data_masked << 5;
    case state_cha_A::sShiftA6:
        return i_cha_A_data_masked << 6;
    case state_cha_A::sShiftA7:
        return i_cha_A_data_masked << 7;
    default:
        return i_cha_A_data_masked;

    }
}

inline uint8_t fb_blitter::get_chaA_explode() {
    if (r_BLTCON_line)
        return r_line_pxmaskcur;
    else
    {
        uint8_t a = get_chaA_shifted2();
        switch (r_BLTCON_mode) {
        case blitter_bppmode::bpp_2:
            return 	(a & 0xF0)
                | ((a & 0xF0) >> 4);
        case blitter_bppmode::bpp_4:
            return 	(a & 0xC0) |
                ((a & 0xC0) >> 2) |
                ((a & 0xC0) >> 4) |
                ((a & 0xC0) >> 6);
        case blitter_bppmode::bpp_8:
            return (a & 0x80) ? 0xFF : 0x00;
        default:
            return a;
        }
    }
}

inline uint8_t fb_blitter::get_chaA_width_in_bytes() {
    switch (r_BLTCON_mode) {
    case blitter_bppmode::bpp_2:
        return 	r_width >> 1;
    case blitter_bppmode::bpp_4:
        return 	r_width >> 2;
    case blitter_bppmode::bpp_8:
        return  r_width >> 3;
    default:
        return r_width;
    }
}

inline uint8_t fb_blitter::get_funcgen_data() {
    uint8_t a = get_chaA_explode();
    uint8_t b = get_chaB_shifted();
    uint8_t r = 0;
    for (uint8_t i = 0x80; i; i = i >> 1) {
        //bits in data
        uint8_t mt = 0x01;
        bool rb = false;
        for (int m = 0; m < 8; m++) {
            //minterms 
            rb |=
                bool(r_FUNCGEN & mt)
                && (bool(a & i) != !bool(m & 4))
                && (bool(b & i) != !bool(m & 2))
                && (bool(r_cha_C_data & i) != !bool(m & 1));

            mt = mt << 1;
        }
        if (rb)
            r |= i;
    }
    return r;
}

inline uint8_t fb_blitter::get_chaB_shifted() {
    switch (r_BLTCON_mode)
    {
    case blitter_bppmode::bpp_1:
        uint8_t rsb;
        rsb = r_shift_B & 0x7;
        return 		(r_cha_B_data_pre << 8 - rsb)
            | (r_cha_B_data >> rsb);
    case blitter_bppmode::bpp_2:
        switch (r_shift_B & 0x3) {
        case 0:
            return r_cha_B_data;
        case 1:
            return ((r_cha_B_data & 0xEE) >> 1)
                | ((r_cha_B_data_pre & 0x11) << 3);
        case 2:
            return ((r_cha_B_data & 0xCC) >> 2)
                | ((r_cha_B_data_pre & 0x33) << 2);
        default:
            return ((r_cha_B_data & 0x88) >> 3)
                | ((r_cha_B_data_pre & 0x77) << 1);
        }
        break;
    case blitter_bppmode::bpp_4:
        if (r_shift_B & 0x1)
            return 	(r_cha_B_data & 0xAA) >> 2 |
            (r_cha_B_data_pre & 0x33) << 2;
        else
            return r_cha_B_data;
    default:
        return r_cha_B_data;
    }
}

inline fb_blitter::state_type fb_blitter::get_state_next() {
    switch (r_blit_state)
    {
    case state_type::sStart:
        if (r_BLTCON_line) {
            return state_type::sLineCalc;
        }
        else if (r_BLTCON_execA) {
            return state_type::sMemAccA;
        }
        else if (r_BLTCON_execC) {
            return state_type::sMemAccC;
        }
        else if (r_BLTCON_execB) {
            return state_type::sMemAccB;
        }
        else if (r_BLTCON_execE) {
            return state_type::sMemAccE;
        }
        else {
            return state_type::sMemAccD;
        }
    case state_type::sLineCalc:
        if (r_BLTCON_execC) {
            return state_type::sMemAccC;
        }
        else if (r_BLTCON_execE) {
            return state_type::sMemAccE;
        }
        else {
            return state_type::sMemAccD;
        }
    case state_type::sMemAccD:
        if (r_BLTCON_line) {
            if (r_width & 0x80) { // width overflowed!
                return state_type::sFinish;
            }
            else if (r_line_minor) {
                if (r_BLTCON_execC) {
                    return state_type::sMemAccC_min;
                }
                else {
                    return state_type::sMemAccD_min;
                }
            }
            else {
                return state_type::sLineCalc;
            }
        }
        else if (r_y_count == 0 && get_main_last()) {
            return state_type::sFinish;
        }
        else if (r_BLTCON_execA && (get_accA_state_next() == state_cha_A::sMemAccA || get_main_last())) {		//TODO: is i_main_last needed here!
            return state_type::sMemAccA;
        }
        else if (r_BLTCON_execC) {
            return state_type::sMemAccC;
        }
        else if (r_BLTCON_execB) {
            return state_type::sMemAccB;
        }
        else if (r_BLTCON_execE) {
            return state_type::sMemAccE;
        }
        else {
            return state_type::sMemAccD;
        }
    case state_type::sMemAccD_min:
        return state_type::sLineCalc;
    case state_type::sMemAccE:
        return state_type::sMemAccD;
    case state_type::sFinish:
        return state_type::sIdle;
    case state_type::sIdle:
        if (r_BLTCON_act) {
            std::cerr << "blit start\n";
            return state_type::sStart;
        }
        else {
            return state_type::sIdle;
        }
    case state_type::sMemAccA:
        if (r_BLTCON_execC) {
            return state_type::sMemAccC;
        }
        else if (r_BLTCON_execB) {
            return state_type::sMemAccB;
        }
        else if (r_BLTCON_execE) {
            return state_type::sMemAccE;
        }
        else {
            return state_type::sMemAccD;
        }
    case state_type::sMemAccC:
        if (r_BLTCON_execB && !r_BLTCON_line) {
            return state_type::sMemAccB;
        }
        else if (r_BLTCON_execE) {
            return state_type::sMemAccE;
        }
        else {
            return state_type::sMemAccD;
        }
    case state_type::sMemAccC_min:
        return state_type::sMemAccD_min;
    case state_type::sMemAccB:
        if (r_BLTCON_execE) {
            return state_type::sMemAccE;
        }
        else {
            return state_type::sMemAccD;
        }
    default:
        return state_type::sFinish;		// finish
    }
}

inline fb_blitter::state_cha_A fb_blitter::get_accA_state_next() {
    state_cha_A ret = r_accA_state_cur;
    if (r_blit_state == state_type::sMemAccA || r_blit_state == state_type::sStart) {
        ret = state_cha_A::sMemAccA;
    }
    else if (r_blit_state == state_type::sMemAccD) {
        if (get_main_last()) {
            ret = state_cha_A::sMemAccA;	// reset at end of line!
        }
        else if (r_BLTCON_mode == blitter_bppmode::bpp_1) {
            ret = state_cha_A::sMemAccA;
        }
        else if (r_BLTCON_mode == blitter_bppmode::bpp_2) {
            if (r_accA_state_cur == state_cha_A::sMemAccA) {
                ret = state_cha_A::sShiftA4;
            }
            else {
                ret = state_cha_A::sMemAccA;
            }
        }
        else if (r_BLTCON_mode == blitter_bppmode::bpp_4) {
            if (r_accA_state_cur == state_cha_A::sMemAccA) {
                ret = state_cha_A::sShiftA2;
            }
            else if (r_accA_state_cur == state_cha_A::sShiftA2) {
                ret = state_cha_A::sShiftA4;
            }
            else if (r_accA_state_cur == state_cha_A::sShiftA4) {
                ret = state_cha_A::sShiftA6;
            }
            else {
                ret = state_cha_A::sMemAccA;
            }
        }
        else {//-- if (r_BLTCON_mode = blitter_bppmode::bpp8 ) {
            if (r_accA_state_cur == state_cha_A::sMemAccA) {
                ret = state_cha_A::sShiftA1;
            }
            else if (r_accA_state_cur == state_cha_A::sShiftA1) {
                ret = state_cha_A::sShiftA2;
            }
            else if (r_accA_state_cur == state_cha_A::sShiftA2) {
                ret = state_cha_A::sShiftA3;
            }
            else if (r_accA_state_cur == state_cha_A::sShiftA3) {
                ret = state_cha_A::sShiftA4;
            }
            else if (r_accA_state_cur == state_cha_A::sShiftA4) {
                ret = state_cha_A::sShiftA5;
            }
            else if (r_accA_state_cur == state_cha_A::sShiftA5) {
                ret = state_cha_A::sShiftA6;
            }
            else if (r_accA_state_cur == state_cha_A::sShiftA6) {
                ret = state_cha_A::sShiftA7;
            }
            else {
                ret = state_cha_A::sMemAccA;
            }
        }
    }
    return ret;
}

inline bool fb_blitter::get_main_last() {
    return r_row_countdn == 0;
}

inline bool fb_blitter::get_cha_A_last() {
    switch (r_BLTCON_mode)
    {
    case blitter_bppmode::bpp_8:
        return !(r_row_countdn & 0xF8);
    case blitter_bppmode::bpp_4:
        return !(r_row_countdn & 0xFC);
    case blitter_bppmode::bpp_2:
        return !(r_row_countdn & 0xFE);
    default:
        return !(r_row_countdn);
    }
}
