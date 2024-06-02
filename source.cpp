#include "packet.h"
#include "Packetizer.h"
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include "date.h"

using namespace std;
using namespace ce;

using namespace date;
using namespace std::chrono;

template <typename T>
inline std::string int_to_hex(T val,size_t width=sizeof(T)*2)
{
std::stringstream ss;
ss<<std::setfill('0')<<std::setw(width)<<std::hex<<(val|0);
return ss.str();	
}

void logTime(ofstream &file){
	auto t= std::time(nullptr);
	auto tm=*std::localtime(&t);
	
	file<<system_clock::now()<<": ";
	}

int main() {
	packet TSIPrxpkt;
	packet TIPYrxpkt;

	ofstream TSIPLOG;
	TSIPLOG.open("/home/pi/Desktop/TSIPLOG.bin",ios::binary|ios::out);
	
	ofstream TIPYREQ;
	TIPYREQ.open("/home/pi/Desktop/TIPY REQ.bin",ios::binary|ios::out);
	
	ofstream TIPYREP;
	TIPYREP.open("/home/pi/Desktop/TIPY REP.bin",ios::binary|ios::out);
	
	ofstream TIPYALL;
	TIPYALL.open("/home/pi/Desktop/TIPY ALL.bin",ios::binary|ios::out);
	
	Packetizer TSIP_hdlr;
	Packetizer TIPY_hdlr;

	uint8_t* tsipPacket = 0;
	uint8_t tsipId=0x00;
	uint8_t tsipPktlen=0;
	bool tsipEnded=false;

	uint8_t* tipyPacket = 0;
	uint8_t tipyId;
	uint8_t tipyPktlen;
	bool tipyEnded = false;

	packet R41pkt;//time and date
	packet R6Dpkt;//receiver mode
	packet R46pkt;//receiver health
	packet R47pkt;//sv signal levels mode
	
	packet R42pkt;//Single-Precision Position Fix, XYZ ECEF
	packet R4Apkt;//Single Precision LLA Position Fix
	packet R43pkt;//Velocity Fix, XYZ ECEF
	packet R56pkt;//Velocity Fix, East-North-Up (ENU)		
	packet R83pkt;//Double-precision XYZ Position Fix and Bias Information
	packet R84pkt;//Double-precision LLA Position Fix and Bias Information
	
	packet R57pkt;//Information About Last Computed Fix	

	packet R4Cpkt;// dynamics code, SV and GOOP mask parameters 
	uint8_t R4CData[17] = { 0x03,0x3E,0x32,0xB5,0x64,0x41,0xC0,0x00,0x00,0x41,0xC0,0x00,0x00,0x41,0x20,0x00,0x00 };
	R4Cpkt.setPacket(TSIP, Response, 0x4C);
	R4Cpkt.load(R4CData, 17);

	packet R55pkt;//I/O Options
	uint8_t R55Data[4] = { 0x01,0x00,0x05,0x08 };
	R55pkt.setPacket(TSIP, Response, 0x55);
	R55pkt.load(R55Data, 4);

	packet R94pkt;//timing  mode
	uint8_t R94Data[1] = {0x07};
	R94pkt.setPacket(TSIP, Response, 0x94);
	R94pkt.load(R94Data, 1);

	packet R97pkt;//BIT Result
	uint8_t R97Data[24];
	for (int i = 0; i < 24; i++)
	{
		R97Data[i] = 0x00;
	}
	R97pkt.setPacket(TSIP, Response, 0x97);
	R97pkt.load(R97Data, 24);

	packet Ra7pkt;//Receiver Modes (The GPS receiver sends packet A7 hex only on power-up and reset (or on request))
	uint8_t Ra7Data[19] = { 0x04,0x00,
							0x0c,0x14,0x5f,
							0x0000,
							0x0002,
							0x00,
							0x07,
							0x02,
							0x01,
							0x0004,
							0x00,
							0x0d};
	Ra7pkt.setPacket(TSIP, Response, 0xa7);
	Ra7pkt.load(Ra7Data, 19);

	packet Rf5pkt;// Boot Version Number ( transmitted whenever power to the GPS receiver is cycled.)
	uint8_t Rf5Data[6] = { 0x30,0x33,0x2e,0x31,0x00,0x00 };
	Rf5pkt.setPacket(TSIP, Response, 0xf5);
	Rf5pkt.load(Rf5Data, 6);




	TSIP_hdlr.open("/dev/ttyAMA2", 1024, -1);
	TIPY_hdlr.open("/dev/ttyUSB0", 1024, -1);

	Rf5pkt.formatTIPY();
	TIPY_hdlr.put(Rf5pkt.FormatedPktData, Rf5pkt.FormatedPktLength);
	
	Ra7pkt.formatTIPY();
	TIPY_hdlr.put(Ra7pkt.FormatedPktData, Ra7pkt.FormatedPktLength);
	
	logTime(TIPYREP);
	TIPYREP<<"packet :";
	for (int i = 0; i < Rf5pkt.FormatedPktLength; i++) {
		TIPYREP<<int_to_hex(Rf5pkt.FormatedPktData[i],2);
	}
	TIPYREP<<endl;
	
	logTime(TIPYREP);
	TIPYREP<<"packet :";
	for (int i = 0; i < Ra7pkt.FormatedPktLength; i++) {
		TIPYREP<<int_to_hex(Ra7pkt.FormatedPktData[i],2);
	}
	TIPYREP<<endl;
	
	logTime(TIPYALL);
	TIPYALL<<"packet txd:";
	for (int i = 0; i < Rf5pkt.FormatedPktLength; i++) {
		TIPYALL<<int_to_hex(Rf5pkt.FormatedPktData[i],2);
	}
	TIPYALL<<endl;	
	
	logTime(TIPYALL);
	TIPYALL<<"packet txd:";
	for (int i = 0; i < Ra7pkt.FormatedPktLength; i++) {
		TIPYALL<<int_to_hex(Ra7pkt.FormatedPktData[i],2);
	}
	TIPYALL<<endl;
	
	
	for (;;)
	{
		TSIP_hdlr.get(&tsipPacket, &tsipPktlen, tsipEnded, &tsipId);
		TIPY_hdlr.get(&tipyPacket, &tipyPktlen, tipyEnded, &tipyId);
		TSIP_hdlr.COM_PORT.Delay(50);

		if (tipyEnded)
		{
			
			TIPYrxpkt.setPacket(TIPY, Command, tipyId);
			TIPYrxpkt.load(tipyPacket, tipyPktlen);

			printf("%0.2X ", tipyId);
			
			logTime(TIPYREP);
			TIPYREQ<<"packet: "<< "10" << int_to_hex(tipyId,2);
			for (int i = 0; i < tipyPktlen; i++) {
				printf("%0.2X ", tipyPacket[i]);
				TIPYREQ<<int_to_hex(tipyPacket[i],2);
			}
			TIPYREQ<< "10" <<"03"<<endl;
			
			logTime(TIPYALL);
			TIPYALL<<"packet rxd:"<< "10" << int_to_hex(tipyId,2);
			for (int i = 0; i < tipyPktlen; i++) {
				TIPYALL<<int_to_hex(tipyPacket[i],2);
			}
			TIPYALL<< "10" <<"03"<<endl;
			
			switch (tipyId)
			{
			case 0x21: //request time-date
				TIPYrxpkt.formatTIPY_ACK_RES(tipyId, tipyPacket, tipyPktlen, 0x41, R41pkt.FormatedPktData, R41pkt.FormatedPktLength);
				TIPY_hdlr.put(TIPYrxpkt.FormatedPktData, TIPYrxpkt.FormatedPktLength);
				
				break;

			case 0x24: //request Receiver's operating MODE, SVs used and DOPs 
				TIPYrxpkt.formatTIPY_ACK_RES(tipyId, tipyPacket, tipyPktlen, 0x44, R6Dpkt.FormatedPktData, R6Dpkt.FormatedPktLength);
				TIPY_hdlr.put(TIPYrxpkt.FormatedPktData, TIPYrxpkt.FormatedPktLength);
				break;

			case 0x25: //Initiate Soft Reset & Self Test
				TIPYrxpkt.formatTSIP();
				TSIP_hdlr.put(TIPYrxpkt.FormatedPktData, TIPYrxpkt.FormatedPktLength);
				//then sends packet 0xA7 (current Receiver modes) and packet 0x41(GPS-Time) according to packet 0x55 (IO Option)

				break;

			case 0x26: //request Receiver health 
				TIPYrxpkt.formatTIPY_ACK_RES(tipyId, tipyPacket, tipyPktlen, 0x46, R46pkt.FormatedPktData, R46pkt.FormatedPktLength);
				TIPY_hdlr.put(TIPYrxpkt.FormatedPktData, TIPYrxpkt.FormatedPktLength);
				break;

			case 0x27: //request SV signal levels 
				//format and send the tsip command and send it
				// as this packet is NOT automatically generated
				TIPYrxpkt.formatTSIP();
				TSIP_hdlr.put(TIPYrxpkt.FormatedPktData, TIPYrxpkt.FormatedPktLength);
				//wait for packet 47 to come from tsip source
				do
				{
					TSIP_hdlr.get(&tsipPacket, &tsipPktlen, tsipEnded, &tsipId);
				} while (tsipId!=0x47);
				R47pkt.setPacket(TSIP, Response, tsipId);
				R47pkt.load(tsipPacket, tsipPktlen);
				R47pkt.formatTIPY();


				TIPYrxpkt.formatTIPY_ACK_RES(tipyId, tipyPacket, tipyPktlen, 0x47, R47pkt.FormatedPktData, R47pkt.FormatedPktLength);
				TIPY_hdlr.put(TIPYrxpkt.FormatedPktData, TIPYrxpkt.FormatedPktLength);
				break;

			case 0x2C: // dynamics code, SV and GOOP mask parameters
				if (tipyPacket[0] == 0x13) {
					for (int i = 0; i < 17; i++)
					{
						R4CData[i] = tipyPacket[i + 1];
					}
					R4Cpkt.load(R4CData, 17);
				}
				R4Cpkt.formatTIPY();
				TIPYrxpkt.formatTIPY_ACK_RES(tipyId, tipyPacket, tipyPktlen, 0x4C, R4Cpkt.FormatedPktData, R4Cpkt.FormatedPktLength);
				TIPY_hdlr.put(TIPYrxpkt.FormatedPktData, TIPYrxpkt.FormatedPktLength);
				break;

			case 0x35: //request Receiver health 
				R55pkt.formatTIPY();
				TIPYrxpkt.formatTIPY_ACK_RES(tipyId, tipyPacket, tipyPktlen, 0x55, R55pkt.FormatedPktData, R55pkt.FormatedPktLength);
				TIPY_hdlr.put(TIPYrxpkt.FormatedPktData, TIPYrxpkt.FormatedPktLength);
				break;
			case 0x74: //SET/Request TIMING mode
				if (tipyPacket[0] == 0x02) {
					//request
					R94pkt.formatTIPY();
					TIPYrxpkt.formatTIPY_ACK_RES(tipyId, tipyPacket, tipyPktlen, 0x94, R94pkt.FormatedPktData, R94pkt.FormatedPktLength);
					TIPY_hdlr.put(TIPYrxpkt.FormatedPktData, TIPYrxpkt.FormatedPktLength);
				}
				else if(tipyPacket[0] == 0x03) {
					//set
					R94Data[0] = tipyPacket[1];
					R94pkt.load(R94Data, 1);
					TIPYrxpkt.formatTIPY_ACK_RES(tipyId, tipyPacket, tipyPktlen, NULL, NULL, NULL);
				}	
				break;
			case 0x77: //BIT result
				R97pkt.formatTIPY();
				TIPYrxpkt.formatTIPY_ACK_RES(tipyId, tipyPacket, tipyPktlen, 0x97, R97pkt.FormatedPktData, R97pkt.FormatedPktLength);
				TIPY_hdlr.put(TIPYrxpkt.FormatedPktData, TIPYrxpkt.FormatedPktLength);
				break;
			case 0x76://BIT result
				R97pkt.formatTIPY();
				TIPYrxpkt.formatTIPY_ACK_RES(tipyId, tipyPacket, tipyPktlen, 0x97, R97pkt.FormatedPktData, R97pkt.FormatedPktLength);
				TIPY_hdlr.put(TIPYrxpkt.FormatedPktData, TIPYrxpkt.FormatedPktLength);
				break;
			default:
				TIPYrxpkt.formatTIPY_ACK_RES(tipyId, tipyPacket, tipyPktlen, NULL, NULL, NULL);
				break;
			}
			logTime(TIPYREP);
			TIPYREP<<"packet :";
			for (int i = 0; i < TIPYrxpkt.FormatedPktLength; i++) {
				TIPYREP<<int_to_hex(TIPYrxpkt.FormatedPktData[i],2);
			}
			TIPYREP<<endl;
			
			logTime(TIPYALL);
			TIPYALL<<"packet txd:";
			for (int i = 0; i < TIPYrxpkt.FormatedPktLength; i++) {
				TIPYALL<<int_to_hex(TIPYrxpkt.FormatedPktData[i],2);
			}
			TIPYALL<<endl;


			
		}
		
		if (tsipEnded) {
			

			TSIPrxpkt.setPacket(TSIP, Response, tsipId);
			TSIPrxpkt.load(tsipPacket, tsipPktlen);
			logTime(TSIPLOG);
			TSIPLOG<<"packet: "<< 10<<int_to_hex(tsipId,2);
			for (int i = 0; i < tsipPktlen; i++)
			{
				TSIPLOG<<int_to_hex(tsipPacket[i],2);
			}
			TSIPLOG<<"10"<<"03";
			TSIPLOG<<endl;
			
			switch (tsipId)
			{
			case 0x41:
				R41pkt.setPacket(TSIP, Response, tsipId);
				R41pkt.load(tsipPacket, tsipPktlen);
				R41pkt.formatTIPY();
				printf("\n");
				for (int i = 0; i < tsipPktlen; i++)
				{
					printf("%0.2X ", tsipPacket[i]);
				}
				TSIP_R41 r41;

				printf("\n");
				if (!R41pkt.get(r41)) {
					printf(" ERR \n");
				}
				else {
					printf("  time      = %f\n", r41.time);
					printf("  week      = %d\n", r41.week);
					printf("  offset    = %f\n", r41.offset);
					
				}
				break;
			case 0x6D:
				R6Dpkt.setPacket(TSIP, Response, tsipId);
				R6Dpkt.load(tsipPacket, tsipPktlen);
				R6Dpkt.formatTIPY();
				printf("\n");

				TSIP_R6D r6D;

				printf("\n");
				if (!R6Dpkt.get(r6D)) {
					printf(" ERR \n");
				}
				else {
					//printf("  fixmod = %02X\n", r6D.fixmode);
					printf("  PDOP   = %f\n", r6D.pdop);
					printf("  HDOP   = %f\n", r6D.hdop);
					printf("  VDOP   = %f\n", r6D.vdop);
					printf("  TDOP   = %f\n", r6D.tdop);
					for (uint8_t ux = 0; ux < r6D.n; ++ux) {
						printf("  SVPRN[%u] = %d\n", ux, r6D.sv_prn[ux]);
					}
				}
				break;
			case 0x46:
				R46pkt.setPacket(TSIP, Response, tsipId);
				R46pkt.load(tsipPacket, tsipPktlen);
				R46pkt.formatTIPY();
				printf("\n");

				TSIP_R46 r46;

				printf("\n");
				if (!R46pkt.get(r46)) {
					printf(" ERR \n");
				}
				else {
					printf("  status = %d\n", int(r46.status));
					switch (r46.status) {
					case DoingPositionFixes:
						puts("  (Doing position fixes)");
						break;
					case DoNotHaveGPSTimeYet:
						puts("  (Do not have GPS time yet)");
						break;
					case PDOPIsTooHigh:
						puts("  (PDOP is too high)");
						break;
					case NoUsableSatellites:
						puts("  (No usable satellites)");
						break;
					case Only1UsableSat:
						puts("  (Only 1 usable satellite)");
						break;
					case Only2UsableSats:
						puts("  (Only 2 usable satellites)");
						break;
					case Only3UsableSats:
						puts("  (Only 3 usable satellites)");
						break;
					case ChosenSatIsUnusable:
						puts("  (Chosen satellite is unusable)");
						break;
					default:
						;
					};
					printf("  error_code = %02X\n", r46.u.error_code);
					printf("  bat failed = %d\n", r46.u.flags.battery_failed);
					printf("  ant fault  = %d\n", r46.u.flags.antenna_fault);
					printf("  exc errors = %d\n", r46.u.flags.excessive_errs);
				}
				break;
			case 0x47:
				R47pkt.setPacket(TSIP, Response, tsipId);
				R47pkt.load(tsipPacket, tsipPktlen);
				R47pkt.formatTIPY();
				printf("\n");

				TSIP_R47 r47;

				printf("\n");
				if (!R47pkt.get(r47)) {
					printf(" ERR \n");
				}
				else {
					//printf("  count = %u\n", r47.count);
					for (unsigned ux = 0; ux < r47.count; ux++) {
						printf("    %02X level %.lf\n",
							r47.sat[ux].prn,
							r47.sat[ux].siglevel);
					}
				}
				break;

			case 0x42:
				printf("42");
				R42pkt.setPacket(TSIP, Response, tsipId);
				R42pkt.load(tsipPacket, tsipPktlen);
				R42pkt.formatTIPY();
				printf("len:  %d  \n", tsipPktlen);
				TIPY_hdlr.put(R42pkt.FormatedPktData, R42pkt.FormatedPktLength);
				
			logTime(TIPYREP);
			TIPYREP<<"packet :";
			for (int i = 0; i < R42pkt.FormatedPktLength; i++) {
				TIPYREP<<int_to_hex(R42pkt.FormatedPktData[i],2);
			}
			TIPYREP<<endl;
			
			logTime(TIPYALL);
			TIPYALL<<"packet txd:";
			for (int i = 0; i < R42pkt.FormatedPktLength; i++) {
				TIPYALL<<int_to_hex(R42pkt.FormatedPktData[i],2);
			}
			TIPYALL<<endl;
			
				break;
			case 0x4A:
				printf("4A");
				R4Apkt.setPacket(TSIP, Response, tsipId);
				R4Apkt.load(tsipPacket, tsipPktlen);
				printf("len:  %d  \n", tsipPktlen);
				R4Apkt.formatTIPY();
				TIPY_hdlr.put(R4Apkt.FormatedPktData, R4Apkt.FormatedPktLength);
			logTime(TIPYREP);
			TIPYREP<<"packet :";
			for (int i = 0; i < R4Apkt.FormatedPktLength; i++) {
				TIPYREP<<int_to_hex(R4Apkt.FormatedPktData[i],2);
			}
			TIPYREP<<endl;
			
			logTime(TIPYALL);
			TIPYALL<<"packet txd:";
			for (int i = 0; i < R4Apkt.FormatedPktLength; i++) {
				TIPYALL<<int_to_hex(R4Apkt.FormatedPktData[i],2);
			}
			TIPYALL<<endl;
				break;
			case 0x43:
				printf("43");
				R43pkt.setPacket(TSIP, Response, tsipId);
				R43pkt.load(tsipPacket, tsipPktlen);
				printf("len:  %d  \n", tsipPktlen);
				R43pkt.formatTIPY();
				TIPY_hdlr.put(R43pkt.FormatedPktData, R43pkt.FormatedPktLength);
			logTime(TIPYREP);
			TIPYREP<<"packet :";
			for (int i = 0; i < R43pkt.FormatedPktLength; i++) {
				TIPYREP<<int_to_hex(R43pkt.FormatedPktData[i],2);
			}
			TIPYREP<<endl;
			
			logTime(TIPYALL);
			TIPYALL<<"packet txd:";
			for (int i = 0; i < R43pkt.FormatedPktLength; i++) {
				TIPYALL<<int_to_hex(R43pkt.FormatedPktData[i],2);
			}
			TIPYALL<<endl;
				break;
			case 0x56:
				printf("56");
				R56pkt.setPacket(TSIP, Response, tsipId);
				R56pkt.load(tsipPacket, tsipPktlen);
				printf("len:  %d  \n", tsipPktlen);
				R56pkt.formatTIPY();
				TIPY_hdlr.put(R56pkt.FormatedPktData, R56pkt.FormatedPktLength);
			logTime(TIPYREP);
			TIPYREP<<"packet :";
			for (int i = 0; i < R56pkt.FormatedPktLength; i++) {
				TIPYREP<<int_to_hex(R56pkt.FormatedPktData[i],2);
			}
			TIPYREP<<endl;
			
			logTime(TIPYALL);
			TIPYALL<<"packet txd:";
			for (int i = 0; i < R56pkt.FormatedPktLength; i++) {
				TIPYALL<<int_to_hex(R56pkt.FormatedPktData[i],2);
			}
			TIPYALL<<endl;
				break;
			case 0x83:
				printf("83");
				R83pkt.setPacket(TSIP, Response, tsipId);
				R83pkt.load(tsipPacket, tsipPktlen);
				printf("len:  %d  \n", tsipPktlen);
				R83pkt.formatTIPY();
				TIPY_hdlr.put(R83pkt.FormatedPktData, R83pkt.FormatedPktLength);
			logTime(TIPYREP);
			TIPYREP<<"packet :";
			for (int i = 0; i < R83pkt.FormatedPktLength; i++) {
				TIPYREP<<int_to_hex(R83pkt.FormatedPktData[i],2);
			}
			TIPYREP<<endl;
			
			logTime(TIPYALL);
			TIPYALL<<"packet txd:";
			for (int i = 0; i < R83pkt.FormatedPktLength; i++) {
				TIPYALL<<int_to_hex(R83pkt.FormatedPktData[i],2);
			}
			TIPYALL<<endl;
				break;
			case 0x84:
				printf("84");
				R84pkt.setPacket(TSIP, Response, tsipId);
				R84pkt.load(tsipPacket, tsipPktlen);
				printf("len:  %d  \n", tsipPktlen);
				R84pkt.formatTIPY();
				TIPY_hdlr.put(R84pkt.FormatedPktData, R84pkt.FormatedPktLength);
			logTime(TIPYREP);
			TIPYREP<<"packet :";
			for (int i = 0; i < R84pkt.FormatedPktLength; i++) {
				TIPYREP<<int_to_hex(R84pkt.FormatedPktData[i],2);
			}
			TIPYREP<<endl;
			
			logTime(TIPYALL);
			TIPYALL<<"packet txd:";
			for (int i = 0; i < R84pkt.FormatedPktLength; i++) {
				TIPYALL<<int_to_hex(R84pkt.FormatedPktData[i],2);
			}
			TIPYALL<<endl;
				break;
			case 0x57:
				printf("57");
				R57pkt.setPacket(TSIP, Response, tsipId);
				R57pkt.load(tsipPacket, tsipPktlen);
				printf("len:  %d  \n", tsipPktlen);
				R57pkt.formatTIPY();
				TIPY_hdlr.put(R57pkt.FormatedPktData, R57pkt.FormatedPktLength);
			logTime(TIPYREP);
			TIPYREP<<"packet :";
			for (int i = 0; i < R57pkt.FormatedPktLength; i++) {
				TIPYREP<<int_to_hex(R57pkt.FormatedPktData[i],2);
			}
			TIPYREP<<endl;
			
			logTime(TIPYALL);
			TIPYALL<<"packet txd:";
			for (int i = 0; i < R57pkt.FormatedPktLength; i++) {
				TIPYALL<<int_to_hex(R57pkt.FormatedPktData[i],2);
			}
			TIPYALL<<endl;
				break;													
			default:
				break;
			}

		}
		
		
		
		
	}
	TSIPLOG.close();
	TIPYREQ.close();
	TIPYREP.close();
	TIPYALL.close();
}
