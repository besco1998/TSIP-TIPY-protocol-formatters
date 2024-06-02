#include "packet.h"
#include <string.h>
#include <stdio.h>

float parse_single(unsigned char* d) {
	uint32_t	i;
	float	f;

	i = ((uint32_t)d[0] << 24)
		| ((uint32_t)d[1] << 16)
		| ((uint32_t)d[2] << 8)
		| (uint32_t)d[3];

	f = *(float*)&i;
	return f;
}


packet::packet() {
	this->PktLength = 0;
	this->PktId = 0x00;
	this->PktProtocol = TSIP;
	this->PktType = Response;
	this->offset = 0x00;
	this->PktData = new uint8_t[MaxPktLen];
	this->FormatedPktData= new uint8_t[MaxPktLen];
	this->FormatedPktLength = 0x00;
}

uint8_t 
packet::checksum(uint8_t* PktData, uint16_t	PktLength) {
	//uint32_t sum = PktLength+2;
	uint32_t sum = 0;
	bool dle_exist = false;
	for (int i = 0; i < PktLength; i++)
	{
		if (PktData[i] == 0x10) {
			dle_exist = true;
			
		}
		if ((PktData[i+1] == 0x10 && dle_exist == true) || PktData[i] != 0x10) {
			dle_exist = false;
			sum += PktData[i];
		}
	}
	sum = ~sum;
	sum += 1;
	return sum;

}


bool 
packet::setPacket(Protocol PktProtocol, PacketType PktType, uint8_t PktId) {
	this->PktProtocol = PktProtocol;
	this->PktType = PktType;
	this->PktId = PktId;
	return true;
}

bool 
packet::pushData(uint8_t byte) {
	this->PktData[PktLength] = byte;
	this->PktLength += 1;
	return true;
}

void
packet::load(uint8_t* buf, uint16_t buflen) {
	//for (int i = 0; i < buflen; i++)
	//{
	//	this->PktData[i]=buf[i];
	//}
	this->PktData=buf;
	PktLength = buflen;
	offset = 0;
}

bool 
packet::formatTIPY() {
	this->FormatedPktLength = 0;
	this->FormatedPktData[FormatedPktLength++] = 0x10;
	switch (this->PktId)
	{
	case 0x6D:
		this->FormatedPktData[this->FormatedPktLength++] = 0x44;//id
		this->FormatedPktData[this->FormatedPktLength++] = 25;//length
		if (((this->PktData[0] >> 3) & 0x01) == 0x00)
		{
			if ((this->PktData[0] & 0x07) == 0x03)
			{
				this->FormatedPktData[this->FormatedPktLength++] = 0x03;
			}
			else if ((this->PktData[0] & 0x07) == 0x04)
			{
				this->FormatedPktData[this->FormatedPktLength++] = 0x04;
			}
		}
		else if (((this->PktData[0] >> 3) & 0x01) == 0x01)
		{
			if ((this->PktData[0] & 0x0F) == 0x0B)
			{
				this->FormatedPktData[this->FormatedPktLength++] = 0x0D;
			}
			else if ((this->PktData[0] & 0x0F) == 0x0C)
			{
				this->FormatedPktData[this->FormatedPktLength++] = 0x0E;
			}
		}
		else
		{
			this->FormatedPktData[this->FormatedPktLength++] = 0x00;
		}
		for (int i = 0; i < 6; i++) {
			this->FormatedPktData[this->FormatedPktLength++] = this->PktData[17+i];
			if (this->PktData[17 + i] == 0x10) {
				this->FormatedPktData[this->FormatedPktLength++] = 0x10;
			}
		}
		for (int i = 0; i < 16; i++) {
			this->FormatedPktData[this->FormatedPktLength++] = this->PktData[1 + i];
			if (this->PktData[1 + i] == 0x10) {
				this->FormatedPktData[this->FormatedPktLength++] = 0x10;
			}
		}
		break;
	case 0x41:
		this->FormatedPktData[this->FormatedPktLength++] = this->PktId;
		this->FormatedPktData[this->FormatedPktLength++] = this->PktLength + 2;
		for (int i = 0; i < (this->PktLength); i++)
		{
			this->FormatedPktData[this->FormatedPktLength++] = this->PktData[i];
		}
		break;

	case 0x46:
		this->FormatedPktData[this->FormatedPktLength++] = this->PktId;
		this->FormatedPktData[this->FormatedPktLength++] = this->PktLength + 2;
		if (this->PktData[0]==0) {
			this->FormatedPktData[this->FormatedPktLength++] = 0;
		}
		else if (this->PktData[0] == 1) {
			this->FormatedPktData[this->FormatedPktLength++] = 3;
		}
		else if (this->PktData[0] == 3) {
			this->FormatedPktData[this->FormatedPktLength++] = 5;
		}
		else if (this->PktData[0] == 4) {
			this->FormatedPktData[this->FormatedPktLength++] = 6;
		}
		else if (this->PktData[0] == 8) {
			this->FormatedPktData[this->FormatedPktLength++] = 7;
		}
		else{
			this->FormatedPktData[this->FormatedPktLength++] = this->PktData[0]-1;
		}

		this->FormatedPktData[this->FormatedPktLength++] = 0;
		break;
	case 0x47:
		this->FormatedPktData[this->FormatedPktLength++] = this->PktId;
		this->FormatedPktData[this->FormatedPktLength++] = (this->PktData[0]*2) + 3;
		this->FormatedPktData[this->FormatedPktLength++] = this->PktData[0];
		for (uint8_t i = 0; i < (this->PktData[0]); i++)
		{
			this->FormatedPktData[this->FormatedPktLength++] = this->PktData[1 + (i * 5)];
			this->FormatedPktData[this->FormatedPktLength++]= static_cast<unsigned char> (parse_single(&this->PktData[(2 + (i * 5))]));
		}
		break;
	case 0x4C:
		this->FormatedPktData[this->FormatedPktLength++] = this->PktId;
		this->FormatedPktData[this->FormatedPktLength++] = this->PktLength + 2;
		for (int i = 0; i < (this->PktLength); i++)
		{
			this->FormatedPktData[this->FormatedPktLength++] = this->PktData[i];
		}
		break;

	case 0x55:
		this->FormatedPktData[this->FormatedPktLength++] = this->PktId;
		this->FormatedPktData[this->FormatedPktLength++] = this->PktLength + 2;
		for (int i = 0; i < (this->PktLength); i++)
		{
			this->FormatedPktData[this->FormatedPktLength++] = this->PktData[i];
		}
		break;
	case 0x94:
		this->FormatedPktData[this->FormatedPktLength++] = this->PktId;
		this->FormatedPktData[this->FormatedPktLength++] = 3;
		this->FormatedPktData[this->FormatedPktLength++] = this->PktData[0];
		break;
	case 0x97:
		this->FormatedPktData[this->FormatedPktLength++] = this->PktId;
		this->FormatedPktData[this->FormatedPktLength++] = this->PktLength + 2;
		for (int i = 0; i < (this->PktLength); i++)
		{
			this->FormatedPktData[this->FormatedPktLength++] = this->PktData[i];
		}
		break;

	case 0xa7:
		this->FormatedPktData[this->FormatedPktLength++] = this->PktId;
		this->FormatedPktData[this->FormatedPktLength++] = this->PktLength + 2;
		for (int i = 0; i < (this->PktLength); i++)
		{
			this->FormatedPktData[this->FormatedPktLength++] = this->PktData[i];
		}
		break;
	case 0xf5:
		this->FormatedPktData[this->FormatedPktLength++] = this->PktId;
		this->FormatedPktData[this->FormatedPktLength++] = this->PktLength + 2;
		for (int i = 0; i < (this->PktLength); i++)
		{
			this->FormatedPktData[this->FormatedPktLength++] = this->PktData[i];
		}
		break;
	
	case 0x57:
		this->FormatedPktData[this->FormatedPktLength++] = this->PktId;
		this->FormatedPktData[this->FormatedPktLength++] = this->PktLength + 2;
		if ((this->PktData[0]) ==0 )
		{
			this->FormatedPktData[this->FormatedPktLength++]==0x09;
		}
		else if ((this->PktData[0]) ==1)
		{
			this->FormatedPktData[this->FormatedPktLength++]==0x16;
		}

		for (int i = 1; i < (this->PktLength); i++)
		{
			this->FormatedPktData[this->FormatedPktLength++] = this->PktData[i];
		}
		break;		
	default:
		
		this->FormatedPktData[this->FormatedPktLength++] = this->PktId;
		this->FormatedPktData[this->FormatedPktLength++] = this->PktLength + 2;
		for (int i = 0; i < (this->PktLength); i++)
		{
			this->FormatedPktData[this->FormatedPktLength++] = this->PktData[i];
		}
		break;	
	}
	this->FormatedPktData[this->FormatedPktLength++] = checksum((this->FormatedPktData)+2, this->FormatedPktLength-2);
	this->FormatedPktData[this->FormatedPktLength++] = 0x10;
	this->FormatedPktData[this->FormatedPktLength++] = 0x03;
	return true;
	
}

bool
packet::formatTSIP() {
	this->FormatedPktLength = 0;
	this->FormatedPktData[this->FormatedPktLength++] = 0x10;
	this->FormatedPktData[this->FormatedPktLength++] = this->PktId;
	/*
		for (int i = 0; i < (this->PktLength); i++)
	{
		this->FormatedPktData[this->FormatedPktLength++] = this->PktData[i];
	}
	*/

	this->FormatedPktData[this->FormatedPktLength++] = 0x10;
	this->FormatedPktData[this->FormatedPktLength++] = 0x03;
	return true;
}
bool 
packet::formatTIPY_ACK_RES(uint8_t cmdID, uint8_t* cmdData, uint8_t cmdlen, uint8_t rspID, uint8_t* rspData, uint16_t rsplen) {
	FormatedPktLength = 0;
	uint8_t cmdStatus = 0x00;
	if (cmdData[0] != cmdlen) {
		cmdStatus = 0x01;
	}
	else if (cmdData[cmdlen-1] != checksum(cmdData, cmdlen-1) )
	{
		cmdStatus = 0x02;
	}
	else {
		cmdStatus = 0x00;
	}

	this->FormatedPktData[this->FormatedPktLength++] = 0x10;
	this->FormatedPktData[this->FormatedPktLength++] = 0xFF;
	this->FormatedPktData[this->FormatedPktLength++] = 0x04;
	this->FormatedPktData[this->FormatedPktLength++] = cmdID;
	this->FormatedPktData[this->FormatedPktLength++] = cmdStatus;
	this->FormatedPktData[this->FormatedPktLength++] = checksum((this->FormatedPktData)+2, 3);
	this->FormatedPktData[this->FormatedPktLength++] = 0x10;
	this->FormatedPktData[this->FormatedPktLength++] = 0x03;


	for (int i = 0; i < rsplen; i++)
	{
		this->FormatedPktData[this->FormatedPktLength++] = rspData[i];
	}




	return true;

}

bool
packet::get(uint8_t& byte) {
	if (offset >= PktLength)
		return false;
	byte = PktData[offset++];
	return true;
}

uint16_t
packet::get(uint8_t* PktData, uint16_t count) {
	uint16_t rcount = 0;

	for (; count > 0 && offset < PktLength; --count, ++rcount)
		*PktData++ = this->PktData[offset++];
	return rcount;
}

bool
packet::get(int16_t& ival) {
	uint8_t bytes[2];

	if (get(bytes, 2) != 2)
		return false;
	ival = ((short)bytes[0] << 8) | (short)bytes[1];
	return true;
}

bool
packet::get(uint16_t& uval) {
	uint8_t bytes[2];

	if (get(bytes, 2) != 2)
		return false;
	uval = ((uint16_t)bytes[0] << 8) | (uint16_t)bytes[1];
	return true;
}

bool
packet::get(int32_t& ival) {
	uint8_t bytes[4];

	if (get(bytes, 4) != 4)
		return false;

	ival = ((int)bytes[0] << 24)
		| ((int)bytes[1] << 16)
		| ((int)bytes[2] << 8)
		| ((int)bytes[3]);
	return true;
}

bool
packet::get(uint32_t& uval) {
	uint8_t bytes[4];

	if (get(bytes, 4) != 4)
		return false;

	uval = ((uint32_t)bytes[0] << 24)
		| ((uint32_t)bytes[1] << 16)
		| ((uint32_t)bytes[2] << 8)
		| ((uint32_t)bytes[3]);
	return true;
}

bool
packet::get(int64_t& ival) {
	int32_t i32;

	if (!get(i32))
		return false;

	ival = (int64_t)i32 << 32;
	if (!get(i32))
		return false;
	ival |= i32;
	return true;
}

bool
packet::get(uint64_t& uval) {
	uint32_t u32;

	if (!get(u32))
		return false;

	uval = (uint64_t)u32 << 32;
	if (!get(u32))
		return false;
	uval |= u32;
	return true;
}

bool
packet::get(float& fval) {
	union {
		int32_t	i32;
		float	f32;
	} u;

	if (!get(u.i32))
		return false;
	fval = u.f32;
	return true;
}

bool
packet::get(double& fval) {
	union {
		int64_t	i64;
		double	f64;
	} u;

	if (!get(u.i64))
		return false;
	fval = u.f64;
	return true;
}

bool
packet::get(TSIP_R41& recd) {
	if (!get(recd.time))
		return false;
	if (!get(recd.week))
		return false;
	if (!get(recd.offset))
		return false;
	return true;
}
bool
packet::get(TSIP_R6D& recd) {
	if (!get(recd.fixmode))
		return false;
	if (!get(recd.pdop))
		return false;
	if (!get(recd.hdop))
		return false;
	if (!get(recd.vdop))
		return false;
	if (!get(recd.tdop))
		return false;

	memset(recd.sv_prn, 0, sizeof recd.sv_prn);
	recd.n = get(recd.sv_prn, sizeof recd.sv_prn);
	return true;
}
bool
packet::get(TSIP_R46& recd) {
	uint8_t b;

	if (!get(b))
		return false;
	recd.status = Status46(b);
	if (!get(recd.u.error_code))
		recd.u.error_code = 0;
	return true;
}
bool
packet::get(TSIP_R47& recd) {

	if (!get(recd.count))
		return false;

	for (uint16_t x = 0; x < recd.count && x < 12; ++x) {
		if (!get(recd.sat[x].prn) || !get(recd.sat[x].siglevel)) {
			recd.count = x;
			return false;
		}
	}
	return true;
}
