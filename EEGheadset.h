/*
  EEGheadset.h - EEG data reading library for Neurosky headset
  Copyright (c) 2013 Vitali Yanushchyk. [mindhack.me] All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef EEGheadset_h
#define EEGheadset_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

 class EEGheadset {
/*
Command Bytes and status supported by Mindwave dongle
 */
	static uint8_t const REQUEST_CONNECT = 0xC0;
	static uint8_t const REQUEST_DISCONNECT = 0xC1;
	static uint8_t const REQUEST_AUTOCONNECT = 0xC2;
	static uint8_t const RESPONSE_CONNECTED = 0xD0;
	static uint8_t const RESPONSE_NOT_FOUND = 0xD1;
	static uint8_t const RESPONSE_DISCONNECTED = 0xD2;
	static uint8_t const RESPONSE_REQUEST_DENIED = 0xD3;
	static uint8_t const RESPONSE_STANDBY_SCAN_MODE = 0xD4;
/*
error messages
*/
	static char* const MSG_ERRORS_NONE;
	static char* const MSG_ERRORS_HEADSET1;
	static char* const MSG_ERRORS_PLENGTH_TOO_LARGE;
	static char* const MSG_ERRORS_CHECKSUM_ERROR;
	static char* const MSG_ERRORS_TRY_CONNECT;
	static char* const MSG_ERRORS_TRY_DISCONNECT;
	public:
		EEGheadset(HardwareSerial &_hsSerial);
/*
useAutoConnect:
True: The dongle connect to any headsets it can find, the dongle will search 
	for headsets for 10 seconds.
False <default>: The dongle connect to headset with 2-byte unique ID, stored 
	in hsID
*/			
		bool useAutoConnect;
		char* hsID;
		void begin(uint32_t baudrate);
/*
Parsing the serial data stream of bytes with brainwave data sent by the MindSet
*/		
		void getData();
/*
TODO: char* serializeJSON();
*/
		char* getError();
/*
Printing packet with brainwave data to hardware port Serial
*/
		void printDataToSerial();
/*
Functions to get last values of brainwave data
*/
		uint8_t getPoorQuality();
		uint8_t getAttention();
		uint8_t getMeditation();
		uint8_t getBlinkStrength();
		uint32_t* getEEGPower();
		uint32_t getDelta();
		uint32_t getTheta();
		uint32_t getLowAlpha();
		uint32_t getHighAlpha();
		uint32_t getLowBeta();
		uint32_t getHighBeta();
		uint32_t getLowGamma();
		uint32_t getMidGamma();
	private:
		static uint16_t const INTERVAL_LAG_MILLISEC = 1000;
		HardwareSerial* hsSerial;
		bool isConnect;
		void clearData();
		void checkConnect();
		uint32_t belayLast;
		uint32_t belayCurrent;
		char* lastError;
		uint8_t generatedChecksum;
		uint8_t checksum; 
		uint8_t poorQuality;
		uint8_t attention;
		uint8_t meditation;
		uint8_t blinkStrength;
		uint8_t eegPowerLength;
		uint32_t eegPower[8];
		void init();
		uint8_t readOneByte();

};

#endif