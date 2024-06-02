#pragma once

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ceSerial.h"
class Packetizer;

typedef void (*cmdcb_t)(Packetizer& pkt, char ch);

class Packetizer {
	const char* device;
	

	enum e_state {
		pkt_idle,
		pkt_data,
		pkt_escape,
		pkt_end
	};

	int	tty_fd;		// Open fd
	uint8_t* buf;		// Packetizer buffer
	int	buflen;		// Current length of buffer
	int	maxlen;		// Max length of buffer
	e_state	state;		// Current state of Packetizer buffer

	cmdcb_t	callback;	// Callback for stdin data

protected:
	enum e_gstate {
		byte_serial,
		byte_stdin,
		byte_timeout,
		byte_eof	// EOF when fd is not a tty, at EOF
	};

	bool	ungot;		// True if we "ungot" a byte
	uint8_t	unbyte;		// The "ungot" byte, if any

	int getb(int fd);			// Read bytte with timeout
	e_gstate getb(uint8_t& byte, int ms);	// Get byte with timeout
	void unget(uint8_t byte);		// Put back a got byte

	void putbuf(uint8_t byte);		// Put byte into buffer (if room)

	

public:	
	ce::ceSerial COM_PORT;
	  Packetizer();
	  ~Packetizer();
	  

	  void open(const char* dev = 0, int maxbuflen = 1024, int fd = -1);
	  inline void registercb(cmdcb_t usrcb) { callback = usrcb; }

	  void putb(uint8_t byte);		// Put byte out to serial port
	  void put(uint8_t* bytes, uint16_t len);	// Put len bytes

	  void get(uint8_t** Packet, int* length, bool& ended, uint8_t* pktID);
};

