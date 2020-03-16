/*B-em v2.2 by Tom Walker
  6502/65c02 host CPU emulation
  
  Dominic Beesley 2020 - separate out main system board functions from 6502.c
  */

#include "m6502.h"
extern "C" {
#include "b-em.h"
#include "sys.h"
#include "adc.h"
#include "disc.h"
#include "i8271.h"
#include "ide.h"
#include "mem.h"
#include "model.h"
#include "mouse.h"
#include "music2000.h"
#include "music5000.h"
#include "paula.h"
#include "serial.h"
#include "scsi.h"
#include "sid_b-em.h"
#include "sound.h"
#include "sysacia.h"
#include "tape.h"
#include "tube.h"
#include "via.h"
#include "sysvia.h"
#include "uservia.h"
#include "vdfs.h"
#include "video.h"
#include "wd1770.h"
}

m6502_device *cpu;

static int dbg_core6502 = 0;

static int dbg_debug_enable(int newvalue) {
    int oldvalue = dbg_core6502;
    dbg_core6502 = newvalue;
    return oldvalue;
};

uint16_t cpu_cur_op_pc;
int nmi, prev_nmi, interrupt;

static int FEslowdown[8] = { 1, 0, 1, 1, 0, 0, 1, 0 };
static int RAMbank[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static uint8_t *memlook[2][256];
static int memstat[2][256];
static int vis20k = 0;

static uint8_t acccon;

static uint16_t buf_remv = 0xffff;
static uint16_t buf_cnpv = 0xffff;
static unsigned char *clip_paste_str, *clip_paste_ptr;
static int os_paste_ch;

static uint32_t do_readmem(uint32_t addr);
static void     do_writemem(uint32_t addr, uint32_t val);


static const char *trap_names[] = { "BRK", NULL };


static uint32_t dbg_disassemble(uint32_t addr, char *buf, size_t bufsize) {
    return dbg6502_disassemble(&core6502_cpu_debug, addr, buf, bufsize, x65c02 ? M65C02 : M6502);
}


static uint32_t dbg_reg_get(int which) {
    switch (which)
    {
    case REG_A:
        return cpu->getA();
    case REG_X:
        return cpu->getX();
    case REG_Y:
        return cpu->getY();
    case REG_S:
        return cpu->getSP();
    case REG_P:
        return 0x30 | cpu->getP();
    case REG_PC:
        return cpu->getPC();
    default:
        log_warn("6502: attempt to get non-existent register");
        return 0;
    }
}

static void dbg_reg_set(int which, uint32_t value) {
    switch (which)
    {
    case REG_A:
        cpu->setA(value);
        break;
    case REG_X:
        cpu->setX(value);
        break;
    case REG_Y:
        cpu->setY(value);
        break;
    case REG_S:
        cpu->setSP(value);
        break;
    case REG_P:
        cpu->setP(value);
        break;
    case REG_PC:
        cpu->setPC(value);
    default:
        log_warn("6502: attempt to get non-existent register");
    }
}

static size_t dbg_reg_print(int which, char *buf, size_t bufsize) {
    switch (which)
    {
    case REG_P:
        return dbg6502_print_flags(cpu->getP(), buf, bufsize);
        break;
    case REG_PC:
    case REG_S:
        return snprintf(buf, bufsize, "%04X", dbg_reg_get(which));
        break;
    default:
        return snprintf(buf, bufsize, "%02X", dbg_reg_get(which));
    }
}

static void dbg_reg_parse(int which, const char *str) {
    uint32_t value = strtol(str, NULL, 16);
    dbg_reg_set(which, value);
}

static uint32_t dbg_disassemble(uint32_t addr, char *buf, size_t bufsize);

static uint32_t dbg_get_instr_addr() {
    return cpu_cur_op_pc;
}


cpu_debug_t core6502_cpu_debug = {
    .cpu_name = "core6502",
    .debug_enable = dbg_debug_enable,
    .memread = do_readmem,
    .memwrite = do_writemem,
    .disassemble = dbg_disassemble,
    .reg_names = dbg6502_reg_names,
    .reg_get = dbg_reg_get,
    .reg_set = dbg_reg_set,
    .reg_print = dbg_reg_print,
    .reg_parse = dbg_reg_parse,
    .get_instr_addr = dbg_get_instr_addr,
    .trap_names = trap_names
};



void os_paste_start(char *str)
{
    /* TODO: DB: reinstate */
    /*
    if (str) {
        if (clip_paste_str)
            free(clip_paste_str);
        clip_paste_str = clip_paste_ptr = (unsigned char *)str;
        os_paste_ch = -1;
        log_debug("6502: paste start, clip_paste_str=%p", clip_paste_str);
    }
    */
}

static void os_paste_remv(void)
{
    /* TODO: DB: reinstate */
    /*
    int ch;

    if (os_paste_ch >= 0) {
        if (p.v)
            a = os_paste_ch;
        else {
            a = y = os_paste_ch;
            os_paste_ch = -1;
        }
    }
    else {
        do {
            ch = *clip_paste_ptr++;
            if (!ch) {
                al_free(clip_paste_str);
                clip_paste_str = clip_paste_ptr = NULL;
                opcode = readmem(pc);
                return;
            }
            if (ch == 0xc2 && *clip_paste_ptr == 0xa3) {
                ch = 0x60; // convert UTF-8 pound into BBC pound.
                clip_paste_ptr++;
            }
            else if (ch == 0x0d && *clip_paste_ptr == 0x0a)
                clip_paste_ptr++;
            else if (ch == 0x0a)
                ch = 0x0d;
        } while (ch >= 128);
        if (p.v)
            a = os_paste_ch = ch;
        else
            a = y = ch;
    }
    p.c = 0;
    opcode = 0x60; // RTS
    */
}

static void os_paste_cnpv(void)
{

    /* TODO: DB: reinstate */
    /*

    if (!p.v && !p.c) {
        int len = strlen((char *)clip_paste_ptr);
        x = len & 0xff;
        y = len >> 8;
        opcode = 0x60; // RTS
        return;
    }
    opcode = readmem(pc);
    */
}

static inline void fetch_opcode(void)
{
    /* TODO: DB: reinstate */
    /*

    pc3 = oldoldpc;
    oldoldpc = oldpc;
    oldpc = pc;
    vis20k = RAMbank[pc >> 12];

    if (dbg_core6502)
        debug_preexec(&core6502_cpu_debug, pc);
    if (pc == buf_remv && x == 0 && clip_paste_ptr)
        os_paste_remv();
    else if (pc == buf_cnpv && x == 0 && clip_paste_ptr)
        os_paste_cnpv();
    else
        opcode = readmem(pc);
    pc++;
    */
}

int tubecycle;

int output = 0;
static int timetolive = 0;

static int cycles;
static int otherstuffcount = 0;
static int romsel;
static int ram4k, ram8k, ram12k, ram20k;

static inline void polltime(int c)
{
    cycles -= c;
    via_poll(&sysvia, c);
    via_poll(&uservia, c);
    video_poll(c, 1);
    otherstuffcount -= c;
    if (motoron) {
        if (fdc_time) {
            fdc_time -= c;
            if (fdc_time <= 0)
                fdc_callback();
        }
        disc_time -= c;
        if (disc_time <= 0) {
            disc_time += 16;
            disc_poll();
        }
    }
    tubecycle += c;
}

static uint32_t do_readmem(uint32_t addr)
{

    if (addr >= 0x10000)
        return 0xFF;

    if (cpu->getPC() == addr)
        fetchc[addr] = 31;
    else
        readc[addr] = 31;

    if (memstat[vis20k][addr >> 8])
        return memlook[vis20k][addr >> 8][addr];
    if (MASTER && (acccon & 0x40) && addr >= 0xFC00)
        return os[addr & 0x3FFF];
    if (addr < 0xFE00 || FEslowdown[(addr >> 5) & 7]) {
        if (cycles & 1) {
            polltime(2);
        }
        else {
            polltime(1);
        }
    }

    if (sound_paula) {
        if (addr >= 0xFCFD && addr <= 0xFDFF) {
            uint8_t r;
            if (paula_read(addr, &r))
                return r;
        }
    }


    switch (addr & ~3) {
    case 0xFC08:
    case 0xFC0C:
        if (sound_music5000)
            return music2000_read(addr);
        break;
    case 0xFC20:
    case 0xFC24:
    case 0xFC28:
    case 0xFC2C:
    case 0xFC30:
    case 0xFC34:
    case 0xFC38:
    case 0xFC3C:
        if (sound_beebsid)
            return sid_read(addr);
        break;

    case 0xFC40:
    case 0xFC44:
    case 0xFC48:
    case 0xFC4C:
    case 0xFC50:
    case 0xFC54:
    case 0xFC58:
        if (scsi_enabled)
            return scsi_read(addr);
        if (ide_enable)
            return ide_read(addr);
        break;

    case 0xFC5C:
        return vdfs_read(addr);
        break;

    case 0xFE00:
    case 0xFE04:
        return crtc_read(addr);

    case 0xFE08:
    case 0xFE0C:
        return acia_read(&sysacia, addr);

    case 0xFE10:
    case 0xFE14:
        return serial_read(addr);

    case 0xFE18:
        if (MASTER)
            return adc_read(addr);
        break;

    case 0xFE24:
    case 0xFE28:
        if (MASTER)
            return wd1770_read(addr);
        break;

    case 0xFE34:
        if (MASTER)
            return acccon;
        break;

    case 0xFE40:
    case 0xFE44:
    case 0xFE48:
    case 0xFE4C:
    case 0xFE50:
    case 0xFE54:
    case 0xFE58:
    case 0xFE5C:
        return sysvia_read(addr);

    case 0xFE60:
    case 0xFE64:
    case 0xFE68:
    case 0xFE6C:
    case 0xFE70:
    case 0xFE74:
    case 0xFE78:
    case 0xFE7C:
        return uservia_read(addr);

    case 0xFE80:
    case 0xFE84:
    case 0xFE88:
    case 0xFE8C:
    case 0xFE90:
    case 0xFE94:
    case 0xFE98:
    case 0xFE9C:
        switch (fdc_type) {
        case FDC_NONE:
        case FDC_MASTER:
            break;
        case FDC_I8271:
            return i8271_read(addr);
        default:
            return wd1770_read(addr);
        }
        break;

    case 0xFEC0:
    case 0xFEC4:
    case 0xFEC8:
    case 0xFECC:
    case 0xFED0:
    case 0xFED4:
    case 0xFED8:
    case 0xFEDC:
        if (!MASTER)
            return adc_read(addr);
        break;

    case 0xFEE0:
    case 0xFEE4:
    case 0xFEE8:
    case 0xFEEC:
    case 0xFEF0:
    case 0xFEF4:
    case 0xFEF8:
    case 0xFEFC:
        return tube_host_read(addr);
    }
    if (addr >= 0xFC00 && addr < 0xFE00)
        return 0xFF;
    return addr >> 8;
}

uint8_t readmem(uint16_t addr)
{
    uint32_t value = do_readmem(addr);
    if (dbg_core6502)
        debug_memread(&core6502_cpu_debug, addr, value, 1);
    return value;
}

static void do_writemem(uint32_t addr, uint32_t val)
{
    int c;

    if (addr < 65536)
        writec[addr] = 31;

    c = memstat[vis20k][addr >> 8];
    if (c == 1) {
        memlook[vis20k][addr >> 8][addr] = val;
        switch (addr) {
        case 0x022c:
            buf_remv = (buf_remv & 0xff00) | val;
            break;
        case 0x022d:
            buf_remv = (buf_remv & 0xff) | (val << 8);
            break;
        case 0x022e:
            buf_cnpv = (buf_cnpv & 0xff00) | val;
            break;
        case 0x022f:
            buf_cnpv = (buf_cnpv & 0xff) | (val << 8);
            break;
        }
        return;
    }
    else if (c == 2) {
        log_debug("6502: attempt to write to ROM %x:%04x=%02x\n", vis20k, addr, val);
        return;
    }
    if (addr < 0xFC00 || addr >= 0xFF00)
        return;
    if (addr < 0xFE00 || FEslowdown[(addr >> 5) & 7]) {
        if (cycles & 1) {
            polltime(2);
        }
        else {
            polltime(1);
        }
    }

    if (sound_music5000) {
        if (addr >= 0xFCFF && addr <= 0xFDFF) {
            music5000_write(addr, val);
            //return -- removed DB need to write to all users of paging register
        }
    }
    if (sound_paula)
    {
        if (addr >= 0xFCFD && addr <= 0xFDFF) {
            paula_write(addr, val);
            return;
        }
    }

    switch (addr & ~3) {
    case 0xFC08:
    case 0xFC0C:
        if (sound_music5000)
            music2000_write(addr, val);
        break;
    case 0xFC20:
    case 0xFC24:
    case 0xFC28:
    case 0xFC2C:
    case 0xFC30:
    case 0xFC34:
    case 0xFC38:
    case 0xFC3C:
        if (sound_beebsid)
            sid_write(addr, val);
        break;

    case 0xFC40:
    case 0xFC44:
    case 0xFC48:
    case 0xFC4C:
    case 0xFC50:
    case 0xFC54:
    case 0xFC58:
        if (scsi_enabled)
            scsi_write(addr, val);
        else if (ide_enable)
            ide_write(addr, val);
        break;

    case 0xFC5C:
        vdfs_write(addr, val);
        break;

    case 0xFE00:
    case 0xFE04:
        crtc_write(addr, val);
        break;

    case 0xFE08:
    case 0xFE0C:
        acia_write(&sysacia, addr, val);
        break;

    case 0xFE10:
    case 0xFE14:
        serial_write(addr, val);
        break;

    case 0xFE18:
        if (MASTER)
            adc_write(addr, val);
        break;

    case 0xFE20:
        videoula_write(addr, val);
        break;

    case 0xFE24:
        if (MASTER)
            wd1770_write(addr, val);
        else
            videoula_write(addr, val);
        break;

    case 0xFE28:
        if (MASTER)
            wd1770_write(addr, val);
        break;

    case 0xFE30:
        ram_fe30 = val;
        for (c = 128; c < 192; c++)
            memlook[0][c] = memlook[1][c] =
            &rom[(val & 15) << 14] - 0x8000;
        for (c = 128; c < 192; c++)
            memstat[0][c] = memstat[1][c] = rom_slots[val & 15].swram ? 1 : 2;
        romsel = (val & 15) << 14;
        ram4k = ((val & 0x80) && MASTER);
        ram12k = ((val & 0x80) && BPLUS);
        RAMbank[0xA] = ram12k;
        if (ram4k) {
            for (c = 128; c < 144; c++)
                memlook[0][c] = memlook[1][c] = ram;
            for (c = 128; c < 144; c++)
                memstat[0][c] = memstat[1][c] = 1;
        }
        if (ram12k) {
            for (c = 128; c < 176; c++)
                memlook[0][c] = memlook[1][c] = ram;
            for (c = 128; c < 176; c++)
                memstat[0][c] = memstat[1][c] = 1;
        }
        break;

    case 0xFE34:
        ram_fe34 = val;
        if (BPLUS) {
            acccon = val;
            vidbank = (val & 0x80) << 8;
            if (val & 0x80)
                RAMbank[0xC] = RAMbank[0xD] = 1;
            else
                RAMbank[0xC] = RAMbank[0xD] = 0;
        }
        if (MASTER) {
            acccon = val;
            ram8k = (val & 8);
            ram20k = (val & 4);
            vidbank = (val & 1) ? 0x8000 : 0;
            if (val & 2)
                RAMbank[0xC] = RAMbank[0xD] = 1;
            else
                RAMbank[0xC] = RAMbank[0xD] = 0;
            for (c = 48; c < 128; c++)
                memlook[0][c] = ram + ((ram20k) ? 32768 : 0);
            if (ram8k) {
                for (c = 192; c < 224; c++)
                    memlook[0][c] = memlook[1][c] =
                    ram - 0x3000;
                for (c = 192; c < 224; c++)
                    memstat[0][c] = memstat[1][c] = 1;
            }
            else {
                for (c = 192; c < 224; c++)
                    memlook[0][c] = memlook[1][c] =
                    os - 0xC000;
                for (c = 192; c < 224; c++)
                    memstat[0][c] = memstat[1][c] = 2;
            }
        }
        break;

    case 0xFE40:
    case 0xFE44:
    case 0xFE48:
    case 0xFE4C:
    case 0xFE50:
    case 0xFE54:
    case 0xFE58:
    case 0xFE5C:
        sysvia_write(addr, val);
        break;

    case 0xFE60:
    case 0xFE64:
    case 0xFE68:
    case 0xFE6C:
    case 0xFE70:
    case 0xFE74:
    case 0xFE78:
    case 0xFE7C:
        uservia_write(addr, val);
        break;

    case 0xFE80:
    case 0xFE84:
    case 0xFE88:
    case 0xFE8C:
    case 0xFE90:
    case 0xFE94:
    case 0xFE98:
    case 0xFE9C:
        switch (fdc_type) {
        case FDC_NONE:
        case FDC_MASTER:
            break;
        case FDC_I8271:
            i8271_write(addr, val);
            break;
        default:
            wd1770_write(addr, val);
        }
        break;

    case 0xFEC0:
    case 0xFEC4:
    case 0xFEC8:
    case 0xFECC:
    case 0xFED0:
    case 0xFED4:
    case 0xFED8:
    case 0xFEDC:
        if (!MASTER)
            adc_write(addr, val);
        break;

    case 0xFEE0:
    case 0xFEE4:
    case 0xFEE8:
    case 0xFEEC:
    case 0xFEF0:
    case 0xFEF4:
    case 0xFEF8:
    case 0xFEFC:
        tube_host_write(addr, val);
        break;
    }
}

void writemem(uint16_t addr, uint8_t val)
{
    if (dbg_core6502)
        debug_memwrite(&core6502_cpu_debug, addr, val, 1);
    do_writemem(addr, val);
}



void sys_reset() {
    int c;
    for (c = 0; c < 16; c++)
        RAMbank[c] = 0;
    for (c = 0; c < 128; c++)
        memstat[0][c] = memstat[1][c] = 1;
    for (c = 128; c < 256; c++)
        memstat[0][c] = memstat[1][c] = 2;
    for (c = 0; c < 128; c++)
        memlook[0][c] = memlook[1][c] = ram;
    if (MODELA) {
        for (c = 0; c < 64; c++)
            memlook[0][c] = memlook[1][c] = ram + 16384;
    }
    for (c = 48; c < 128; c++)
        memlook[1][c] = ram + 32768;
    for (c = 128; c < 192; c++)
        memlook[0][c] = memlook[1][c] = rom - 0x8000;
    for (c = 192; c < 256; c++)
        memlook[0][c] = memlook[1][c] = os - 0xC000;
    memstat[0][0xFC] = memstat[0][0xFD] = memstat[0][0xFE] = 0;
    memstat[1][0xFC] = memstat[1][0xFD] = memstat[1][0xFE] = 0;

    cycles = 0;
    ram4k = ram8k = ram12k = ram20k = 0;

    output = 0;
    tubecycle = tubecycles = 0;
    interrupt = 0;
    nmi = 0;
    prev_nmi = 0;

    //TODO: properly - only works for model B
    if (cpu)
        delete cpu;

    cpu = new m6502_device();
    cpu->init();
    cpu->reset();

}


static void otherstuff_poll(void) {
    otherstuffcount += 128;
    acia_poll(&sysacia);
    if (sound_music5000)
        music2000_poll();
    sound_poll();
    if (!tapelcount) {
        tape_poll();
        tapelcount = tapellatch;
    }
    tapelcount--;
    if (motorspin) {
        motorspin--;
        if (!motorspin)
            fdc_spindown();
    }
    if (ide_count) {
        ide_count -= 200;
        if (ide_count <= 0)
            ide_callback();
    }
    if (adc_time) {
        adc_time--;
        if (!adc_time)
            adc_poll();
    }
    mcount--;
    if (!mcount) {
        mcount = 6;
        mouse_poll();
    }
}

void sys_exec() {


    cycles += 40000;

    while (cycles > 0)
    {
        if (cpu)
        {

            //TODO: check whether the way I did it in beebem is quicker
            if (nmi && !prev_nmi)
            {
                cpu->execute_set_input(M6502_NMI_LINE, ASSERT_LINE);
            }
            prev_nmi = nmi;

            cpu->execute_set_input(M6502_IRQ_LINE, interrupt ? ASSERT_LINE : CLEAR_LINE);

            cpu->tick();

            uint16_t a = cpu->getADDR();

            if (cpu->get_sync())
                cpu_cur_op_pc = a;
            if (cpu->getRNW())
                cpu->setDATA(do_readmem(a));
            else
                do_writemem(a, cpu->getDATA());
        }

        polltime(1);


                //TODO: less?
                if (otherstuffcount <= 0)
                    otherstuff_poll();
                if (tube_exec && tubecycle) {
                        tubecycles += (tubecycle * tube_multipler) >> 1;
                        if (tubecycles > 3)
                                tube_exec();
                        tubecycle = 0;
                }

    }
}


void m6502_savestate(FILE * f)
{
/*        uint8_t temp;
        putc(a, f);
        putc(x, f);
        putc(y, f);
        temp = pack_flags(0x30);
        putc(temp, f);
        putc(s, f);
        putc(pc & 0xFF, f);
        putc(pc >> 8, f);
        putc(nmi, f);
        putc(interrupt, f);
        putc(cycles, f);
        putc(cycles >> 8, f);
        putc(cycles >> 16, f);
        putc(cycles >> 24, f);
        */
}

void m6502_loadstate(FILE * f)
{
    /*
        uint8_t temp;
        a = getc(f);
        x = getc(f);
        y = getc(f);
        temp = getc(f);
        unpack_flags(temp);
        s = getc(f);
        pc = getc(f);
        pc |= (getc(f) << 8);
        nmi = getc(f);
        interrupt = getc(f);
        cycles = getc(f);
        cycles |= (getc(f) << 8);
        cycles |= (getc(f) << 16);
        cycles |= (getc(f) << 24);
    */
}

