#ifndef _FISHBONE_H
#define _FSHBONE_H

#pragma once

#include <stdint.h>


typedef enum fb_cyc_speed {
	MHZ_1	= 0x00,
	MHZ_2	= 0x01,
	MHZ_4	= 0x02,
	MHZ_8	= 0x03,
	MHZ_16	= 0x04

} fb_cyc_speed;

typedef enum fb_cyc_action {
	start,
	stop
} fb_cyc_action;

typedef enum fb_ack {
	ack,
	nul
} fb_ack;






#endif