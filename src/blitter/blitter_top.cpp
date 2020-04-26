#include "blitter_top.h"

blit_SLAVE_NO blitter_top::addr2slaveno(uint32_t addr) {
	addr = addr & 0x00FFFFFF;
	if (addr == 0xFFFE30) {
		// ROMPG on sys always passed to fb_SYS
		return SLAVE_NO_SYS;
	}
	else if (addr == 0xFFFE36) {
		// EEPROM i2c
		return SLAVE_NO_EEPROM;
	}
	else if ((addr & 0xFFFFF0) == 0xFFFE30 && (addr & 0x0F) != 0x4) {
		// other memctl regs FE31-FE37
		return SLAVE_NO_MEMCTL;
	}
	else if ((addr & 0xFFFFF0) == 0xFEFC90) {
		// FF FC9x DMA
		return SLAVE_NO_DMA;
	}
	else if ((addr & 0xFFFFF0) == 0xFEFC80) {
		// FF FC8x SOUND
		return SLAVE_NO_SOUND;
	}
	else if ((addr & 0xFFFFF0) == 0xFEFC60 
		|| (addr & 0xFFFFF0)   == 0xFEFC70 
		|| (addr & 0xFFFFF0)   == 0xFEFCA0) {
		// FF FC6x, FF FC7x, FF FCAx BLITTER
		return SLAVE_NO_BLIT;
	}
	else if ((addr & 0xFFFFF0) == 0xFEFCB0) {
		// FF FC0x - AERIS
		return SLAVE_NO_AERIS;
	}
	else if ((addr == 0xFFFCFD || addr == 0xFFFCFE) && get_JIMEN()) {
		// jim extended paging registers
		// note FCFF is a special case and is always passed on to SYS!
		return SLAVE_NO_JIMCTL;
	}
	else if ((addr & 0xFF0000) == 0xFF0000) {
		// FF xxxx SYS
		return SLAVE_NO_SYS;
	}
	else if (addr >= 0x800000 && addr <= 0xFE0000) {
		// flash memory
		return SLAVE_NO_CHIPFLASH;
	}
	else {
		// everything else to chipram
		return SLAVE_NO_CHIPRAM;
	}
}

#include <fstream>

void blitter_top::init()
{

	intcon.getMas(SLAVE_NO_JIMCTL)->init(jimctl);
	jimctl.init(*intcon.getMas(SLAVE_NO_JIMCTL));

	intcon.getMas(SLAVE_NO_MEMCTL)->init(memctl);
	memctl.init(*intcon.getMas(SLAVE_NO_MEMCTL));

	intcon.getMas(SLAVE_NO_CHIPRAM)->init(chipram);
	chipram.init(*intcon.getMas(SLAVE_NO_CHIPRAM));

	intcon.getMas(SLAVE_NO_CHIPFLASH)->init(flash);
	flash.init(*intcon.getMas(SLAVE_NO_CHIPFLASH));
	
	intcon.getMas(SLAVE_NO_SYS)->init(sys);
	sys.init(*intcon.getMas(SLAVE_NO_SYS));

	intcon.getMas(SLAVE_NO_SOUND)->init(paula.getSlave());
	paula.getSlave().init(*intcon.getMas(SLAVE_NO_SOUND));

	intcon.getMas(SLAVE_NO_DMA)->init(dma.getSlave());
	dma.getSlave().init(*intcon.getMas(SLAVE_NO_DMA));

	intcon.getMas(SLAVE_NO_BLIT)->init(blitter.getSlave());
	blitter.getSlave().init(*intcon.getMas(SLAVE_NO_BLIT));


	intcon.getSla(MAS_NO_PAULA)->init(paula.getMaster());
	paula.getMaster().init(*intcon.getSla(MAS_NO_PAULA));

	intcon.getSla(MAS_NO_DMA)->init(dma.getMaster());
	dma.getMaster().init(*intcon.getSla(MAS_NO_DMA));

	intcon.getSla(MAS_NO_BLIT)->init(blitter.getMaster());
	blitter.getMaster().init(*intcon.getSla(MAS_NO_BLIT));

	intcon.getSla(MAS_NO_CPU)->init(cpu);
	cpu.init(*intcon.getSla(MAS_NO_CPU));

	paula.init();

}

void blitter_top::tick_int(bool sysCycle) {

    count_ticks++;

	sys.tick(sysCycle);
	cpu.tick(sysCycle);
	paula.tick(sysCycle);
	dma.tick(sysCycle);
	blitter.tick(sysCycle);
	flash.tick(sysCycle);
}

bool blitter_top::tick()
{
	return tick((int)0);
}

bool blitter_top::tick(int skip)
{
	while (skip-- > 0)
	{
		// skip sys cycle - the bbc is having a stretch
		tick_int(false);
		tick_int(false);
		tick_int(false);
		tick_int(false);
	}
	tick_int(true);
	tick_int(false);
	tick_int(false);
	tick_int(false);
	return true;
}

void blitter_top::execute_set_input(int inputnum, int state)
{
	
	switch (inputnum) {
	case IRQ_LINE:
		setIRQ(compno_SYS, state == ASSERT_LINE);
		break;
	case HALT_LINE:
		setHALT(compno_SYS, state == ASSERT_LINE);
		break;
	case NMI_LINE:
		setNMI(compno_SYS, state == ASSERT_LINE);
		break;
	}

	cpu.execute_set_input(inputnum, state);
}

void blitter_top::reset()
{
	reg_jimEn = false;
	reg_jimPage = 0;
	reg_ROMPG = 0;

	bits_halt = 0;
	bits_irq = 0;
	bits_nmi = 0;
	
	cpu.reset();

	sys.reset();
	memctl.reset();
	chipram.reset();
	flash.reset();
	jimctl.reset();
	paula.reset();
	dma.reset();
	blitter.reset();

	intcon.reset();

}

void blitter_top::powerReset() {
	reg_blturbo = 0;

}

void blitter_top::setIRQ(int levelno, bool assert)
{
	if (assert)
		bits_irq |= 1 << levelno;
	else
		bits_irq &= ~(1 << levelno);

	cpu.execute_set_input(IRQ_LINE, (bits_irq != 0) ? ASSERT_LINE : CLEAR_LINE);
}

void blitter_top::setNMI(int levelno, bool assert)
{
	if (assert)
		bits_nmi |= 1 << levelno;
	else
		bits_nmi &= ~(1 << levelno);

	cpu.execute_set_input(NMI_LINE, (bits_nmi != 0) ? ASSERT_LINE : CLEAR_LINE);
}

void blitter_top::setHALT(int levelno, bool assert)
{
	if (assert)
		bits_halt |= 1 << levelno;
	else
		bits_halt &= ~(1 << levelno);

	cpu.execute_set_input(HALT_LINE, (bits_halt != 0) ? ASSERT_LINE : CLEAR_LINE);
}

uint32_t blitter_top::log2phys(uint32_t ain) {
	bool map0n1 = true;	//TODO: choose processor

//	A_o <=
//		x"8D3F" & A_i(7 downto 0)
//		when A_i(23 downto 8) = x"0000" and m68k_boot_i = '1'
	if ((ain & 0xFF0000) == 0xFF0000)
	{
		uint16_t ain16 = (uint16_t)ain;
		if (ain16 >= 0x8000 && ain16 < 0xC000 && get_cfg_swram_en())
		{
			if ((reg_ROMPG & 0x0F) < 4 || (reg_ROMPG & 0x0F) >= 8) {

				if (map0n1) {
					if (reg_ROMPG & 0x01) {
						//SWROM from eerpom 8E 0000 - 8F FFFF
						return 0x8E0000 + ((reg_ROMPG & 0x0E) << 13) + (ain & 0x3FFF);
					}
					else {
						//SWRAM from chipram 7E 0000 - 7F FFFF
						return 0x7E0000 + ((reg_ROMPG & 0x0E) << 13) + (ain & 0x3FFF);

					}
				}
				else {
					if (reg_ROMPG & 0x01) {
						//SWROM from eerpom 8C 0000 - 8D FFFF
						return 0x8C0000 + ((reg_ROMPG & 0x0E) << 13) + (ain & 0x3FFF);
					}
					else {
						//SWRAM from chipram 7C 0000 - 7D FFFF
						return 0x7C0000 + ((reg_ROMPG & 0x0E) << 13) + (ain & 0x3FFF);

					}

				}
			}
		}
		else if ((ain16 >= 0xC000 && ain16 < 0xFC00) || (ain16 >= 0xFF00)) {
			// MOS
			
			if (get_nioice_debug_shadow()) {
				if ((ain16 & 0xF000) == 0xC000) {
					//NOICE shadow ram from hidden slot #4 of map 0
					return 0x7E8000 + ain16 & 0x0FFF;
				}
				else {
					if (map0n1)
						return 0x9FC000 + ain16 & 0x3FFF; //NOICE shadow MOS from slot #F map 0
					else
						return 0x9DC000 + ain16 & 0x3FFF; //NOICE shadow MOS from slot #F map 1
				}
			}
			else if (get_swmos_shadow()) {
				if (map0n1)
					return 0x7F0000 + ain16 & 0x3FFF; //SWMOS from slot #8 map 0
				else
					return 0x7D0000 + ain16 & 0x3FFF; //SWMOS from slot #8 map 1
			}
			else if (!map0n1) {
				// normal mos map 1 from slot 9
				return 0x9D0000 + ain16 & 0x3FFF; //SWMOS from slot #8 map 1
			}
		}
		else if (ain16 < 0x8000) {
			if (get_blturbo() & (1 << ((ain16 & 0x7000) >> 12)))
				return ain16 & 0x7FFF;
		}
		else if ((ain16 & 0xFF00) == 0xFD00 && get_JIMEN()) {
			return (((uint32_t)get_JIMPAGE()) << 8) | (uint32_t)(ain16 & 0xFF);

		}
	}

	return ain & 0xFFFFFF;
}


void blitter_top::sound_fillbuf(int16_t *buffer, int len) {
	paula.sound_fillbuf(buffer, len);
}


uint8_t blitter_top::peek(uint16_t addr) {
    uint32_t addrp = log2phys(0x00FF0000 | addr);
    return intcon.getSla(MAS_NO_CPU)->peek(addrp);
}

void blitter_top::poke(uint16_t addr, uint8_t dat) {
    uint32_t addrp = log2phys(0x00FF0000 | addr);
    return intcon.getSla(MAS_NO_CPU)->poke(addrp, dat);
}
