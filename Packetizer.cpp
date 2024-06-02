
//////////////////////////////////////////////////////////////////////
// ttyio.cpp -- Serial I/O Module 
// Date: tue APR 04 2023   (C) Mohamed Ashraf
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <assert.h>
#include "Packetizer.h"
#include "ceSerial.h"


using namespace ce;



void
Packetizer::open(const char* dev, int maxbuflen, int fd) {
	/*
		if (!dev)
		dev = "\\\\.\\COM25";
	*/

	if (maxbuflen <= 0)
		maxbuflen = 1024;

	buf = new uint8_t[maxbuflen + 1];

	maxlen = maxbuflen;
	buflen = 0;
	state = pkt_idle;
	ungot = false;
	unbyte = 0;
	tty_fd = -1;

	this->COM_PORT.SetPort(dev);
	this->COM_PORT.SetBaudRate(9600);
	this->COM_PORT.SetDataSize(8);
	this->COM_PORT.SetParity('N');
	this->COM_PORT.SetStopBits(1);

	printf("Opening Input port %s.\n", this->COM_PORT.GetPort().c_str());
	if (this->COM_PORT.Open() == 0) {
		printf("OK.\n");
	}
	else {
		printf("Error.\n");
	}
}

Packetizer::Packetizer() {
	device = 0;
	tty_fd = -1;
	buf = 0;
	buflen = 0;
	maxlen = 0;
	state = pkt_idle;
	ungot = false;
	unbyte = 0x00;
}

Packetizer::~Packetizer() {
	printf("Closing port %s.\n", this->COM_PORT.GetPort().c_str());
	this->COM_PORT.Close();
	tty_fd = -1;
	if (buf)
		delete buf;
	buf = 0;
}

//////////////////////////////////////////////////////////////////////
// Read a byte from fd
//////////////////////////////////////////////////////////////////////

int
Packetizer::getb(int fd) {
	uint8_t byte;
	bool successFlag;

	byte = this->COM_PORT.ReadChar(successFlag);
	return (int)byte;
}

//////////////////////////////////////////////////////////////////////
// Get a byte from serial port or stdin
// 
// RETURNS:
// 	byte_serial	- Serial byte returned
// 	byte_stdin	- Stdin byte returned
//	byte_timeout	- No byte (timed out)
//	byte_eof	- EOF when not a tty and at EOF
// 
//////////////////////////////////////////////////////////////////////

Packetizer::e_gstate
Packetizer::getb(uint8_t& byte, int ms) {
	/*
			if (ungot) {
		byte = unbyte;
		ungot = false;
		return byte_serial;
		}
	*/

	

	int b = getb(tty_fd);
	if (b == -1)
		return byte_eof;
	byte = (uint8_t)b & 0xFF;
	return byte_serial;
}

//////////////////////////////////////////////////////////////////////
// Put back a read-ahead byte
//////////////////////////////////////////////////////////////////////

void
Packetizer::unget(uint8_t byte) {
	//assert(!ungot);
	ungot = true;
	unbyte = byte;
}

//////////////////////////////////////////////////////////////////////
// Write one byte out to serial port
//////////////////////////////////////////////////////////////////////

void
Packetizer::putb(uint8_t byte) {
	bool successFlag;
	//successFlag = this->COM_PORT.Write(byte,1);
	successFlag = this->COM_PORT.WriteChar(byte);

}

//////////////////////////////////////////////////////////////////////
// Put n bytes
//////////////////////////////////////////////////////////////////////

void
Packetizer::put(uint8_t* buf, uint16_t len) {
	/*
	for (; len > 0; --len)
		putb(*buf++);
	*/
	bool successFlag;
	for (int i = 0; i < len; i++)
	{
		//successFlag = this->COM_PORT.Write(buf[i],1);
		successFlag = this->COM_PORT.WriteChar(buf[i]);
		printf("%0.2x ", buf[i]);
	}
	printf("\n");
	
}


//////////////////////////////////////////////////////////////////////
// Put a byte into the receiving buffer
//////////////////////////////////////////////////////////////////////

void
Packetizer::putbuf(uint8_t byte) {

	assert(buflen > 0 || byte != 0x10);

	if (buflen >= maxlen) {	// discard Packetizer -- too long
		state = pkt_idle;
		buflen = 0;
	}
	else {
		buf[buflen++] = byte;
	}
}

//////////////////////////////////////////////////////////////////////
// Return a Packet
//////////////////////////////////////////////////////////////////////

void
Packetizer::get(uint8_t** Packet, int* length, bool& ended,uint8_t* pktID) {
	uint8_t byte=0x00;
	e_gstate e;

	bool id=false;

	buflen = 0;
	ended = false;

	//////////////////////////////////////////////////////////////
	// First wait for arrival of 0x10 (DLE)
	//////////////////////////////////////////////////////////////

	do {
		e = getb(byte, 250);

		switch (e) {
		case byte_eof:
			*length = 0;
			ended = false;
			return;
		case byte_serial:
			switch (state) {
			case pkt_idle:
				if (byte != 0x10) {
					*length = 0;
					ended = false;
					return;
				}
				if (byte == 0x10) {
					state = pkt_data;
					buflen = 0;
				}
				break;
			case pkt_data:
				if (byte == 0x10) {
					if (buflen > 0)
						state = pkt_escape;
				}
				else if (buflen == 0 && id == false) {
					*pktID = byte;
					id = true;
				}
				else {
					putbuf(byte);
				}
				break;
			case pkt_escape:
				if (byte == 0x10) {
					putbuf(byte);
					state = pkt_data;
				}
				else if (byte == 0x03) {
					state = pkt_end;
					ended = true;
				}
				else {
					unget(byte);
					state = pkt_idle;
				}
				break;
			case pkt_end:
				assert(0);
			}
			break;
		case byte_timeout:
			switch (state) {
			case pkt_idle:
				break;
			case pkt_data:
			case pkt_escape:
				state = pkt_end;
				break;
			case pkt_end:
				assert(0);
			}
			break;
		}
	} while (state!= pkt_end);

	*Packet = buf;
	*length = buflen;
	state = pkt_idle;
}

// End ttyio.cpp

