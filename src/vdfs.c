
#include "b-em.h"
#include <stdbool.h>
#include <stdio.h>

#include "vdfs.h"


/* TODO: reinstate VDFS

Dominic Beesley 2020 - removed during Blitter port, the way VDFS intimately 
uses the 6502 registers is not really compatible

*/


bool vdfs_enabled;

void vdfs_init(void) {

}
void vdfs_reset(void) {

}
void vdfs_close(void) {

}


const char *vdfs_get_root(void) {

    return "/";
}
void vdfs_set_root(const char *dir) {

}

void vdfs_loadstate(FILE *f) {

}
void vdfs_savestate(FILE *f) {

}

uint8_t vdfs_read(uint16_t addr) {
    return 0xFF;
}

void vdfs_write(uint16_t addr, uint8_t value) {

}
