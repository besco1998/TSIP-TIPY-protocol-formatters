#pragma once

#include <stdint.h>
#include "TSIP.h"

#define MaxPktLen 1024
enum Protocol {
	TSIP,
	TIPY
};
enum PacketType {
	Command,
	Response,
	ACK
};


class packet
{
	Protocol PktProtocol;
	PacketType PktType;
	uint8_t PktId;
	uint8_t* PktData;		// receiving buffer
	uint16_t	PktLength;		// current buffer length
	uint16_t	offset;		// Extraction offset

	uint8_t checksum(uint8_t* PktData, uint16_t	PktLength);
	

public:
	uint8_t* FormatedPktData;
	uint16_t	FormatedPktLength;
	packet();
	bool pushData(uint8_t byte);
	bool setPacket(Protocol PktProtocol, PacketType PktType, uint8_t PktId);
	void load(uint8_t* buf, uint16_t buflen);

	bool formatTSIP();
	bool formatTIPY();
	bool formatTIPY_ACK_RES(uint8_t cmdID, uint8_t* cmdData, uint8_t cmdlen, uint8_t rspID, uint8_t* rspData, uint16_t rsplen);

	bool get(uint8_t& byte);
	uint16_t get(uint8_t* buf, uint16_t count);

	bool get(int16_t& ival);
	bool get(int32_t& ival);
	bool get(int64_t& ival);

	bool get(uint16_t& uval);
	bool get(uint32_t& uval);
	bool get(uint64_t& uval);

	bool get(float& fval);
	bool get(double& fval);

	bool get(TSIP_R41& recd);
	bool get(TSIP_R6D& recd);
	bool get(TSIP_R46& recd);
	bool get(TSIP_R47& recd);
};

