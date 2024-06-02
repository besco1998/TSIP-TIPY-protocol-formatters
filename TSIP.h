#pragma once
#include <stdint.h>
//////////////////////////////////////////////////////////////////////
// Response 41 : GPS Time 
//////////////////////////////////////////////////////////////////////

struct TSIP_R41 {
	float	time;		//  GPS time of week (seconds) 
	int16_t	week;		//  GPS week number (weeks) 
	float	offset;		//  UTC/GPS time offset 
};

//////////////////////////////////////////////////////////////////////
// Response 6D : All-In-View Satellite Selection 
//////////////////////////////////////////////////////////////////////
struct TSIP_R6D {
	uint8_t	fixmode;	//  GPS postion fix mode 
	float	pdop;		//  Precision Dilution of Precision 
	float	hdop;		//  Horizontal dilution 
	float	vdop;		//  Vertical dilution 
	float	tdop;		//  Time dilution 
	uint8_t n;		//  # of entries in sv_prn[]
	uint8_t	sv_prn[33];	//  Pseudorandom number (0-32) of first sat in view 
};

//////////////////////////////////////////////////////////////////////
// Response 47 : Signal Levels for All 
//////////////////////////////////////////////////////////////////////

struct TSIP_R47 {
	uint8_t	count;		//  Number of satellite records in packet 
	struct {
		uint8_t	prn;		//  PRN number of first satellite 
		float	siglevel;	//  Signal level of first satellite 
	} sat[12];
};

//////////////////////////////////////////////////////////////////////
// Response 46 : Health of Receiver 
//////////////////////////////////////////////////////////////////////

enum Status46 {
	DoingPositionFixes = 0x00,
	DoNotHaveGPSTimeYet = 0x01,
	PDOPIsTooHigh = 0x03,
	NoUsableSatellites = 0x08,
	Only1UsableSat = 0x09,
	Only2UsableSats = 0x0A,
	Only3UsableSats = 0x0B,
	ChosenSatIsUnusable = 0x0C
};

struct TSIP_R46 {
	Status46 	status;		//  Current status of receiver (flags) 
	union {
		uint8_t	error_code;	//  Error status (flags) 
		struct {
			uint8_t	battery_failed : 1;
			uint8_t	reserved1 : 3;
			uint8_t	antenna_fault : 1;
			uint8_t	excessive_errs : 1;
			uint8_t reserved2 : 2;
		} flags;
	} u;
};