#ifndef __VDFS_H_
#define __VDFS_H_


/* TODO: reinstate VDFS

Dominic Beesley 2020 - removed during Blitter port, the way VDFS intimately 
uses the 6502 registers is not really compatible

*/

extern void vdfs_init(void);
extern void vdfs_reset(void);
extern void vdfs_close(void);
extern uint8_t vdfs_read(uint16_t addr);
extern void vdfs_write(uint16_t addr, uint8_t value);

extern bool vdfs_enabled;
extern const char *vdfs_get_root(void);
extern void vdfs_set_root(const char *dir);

extern void vdfs_loadstate(FILE *f);
extern void vdfs_savestate(FILE *f);


#endif