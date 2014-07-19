/*
 EEGheadset.cpp - EEG data reading library for Neurosky headset
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
#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "EEGheadset.h"

EEGheadset::EEGheadset(HardwareSerial &_hsSerial) {
	hsSerial = &_hsSerial;
	init();
}

void EEGheadset::init() {
	useAutoConnect = false;
	hsID = "";
	clearData();
}

void EEGheadset::clearData() {
	lastError = MSG_ERRORS_NONE;
	poorQuality = 200;
	generatedChecksum = 0;
	checksum = 0; 
	attention = 0;
	meditation = 0;
	blinkStrength = 0;
	eegPowerLength = 0;
	for(uint8_t i=0;i<8;i++) eegPower[i] = 0;
	isConnect = false;	
}

void EEGheadset::begin(uint32_t baudrate) {
	hsSerial->begin(baudrate);
}

void EEGheadset::checkConnect() {
  if (isConnect) {
//Disconnect
    belayLast = millis();
    while(!hsSerial->available()) {
      belayCurrent = millis();
      if ((belayCurrent-belayLast)>INTERVAL_LAG_MILLISEC) {
        sprintf(lastError, MSG_ERRORS_TRY_DISCONNECT);
	    hsSerial->write(REQUEST_DISCONNECT);
        while (!(hsSerial->read()==RESPONSE_DISCONNECTED));
		clearData();
        return;
      } //if
    } //while
    // sprintf(lastError, MSG_ERRORS_NONE);
  } //if (isConnect)
  if (!isConnect) {
//Connect
    belayLast = millis();
	if(!useAutoConnect) {
		if(hsID!="") {
		    hsSerial->write(REQUEST_CONNECT);
		    hsSerial->write(hsID);
		} else sprintf(lastError, MSG_ERRORS_HEADSET1);
	} else hsSerial->write(REQUEST_AUTOCONNECT);

    while (!(hsSerial->read()==RESPONSE_CONNECTED)) {
        belayCurrent = millis();
        if ((belayCurrent-belayLast)>INTERVAL_LAG_MILLISEC) {
        	sprintf(lastError, MSG_ERRORS_TRY_CONNECT);
          	return;
        }
    } //while
    // sprintf(lastError, MSG_ERRORS_NONE);
    isConnect = true;
  } //if (!isConnect)
}

uint8_t EEGheadset::readOneByte() {
	checkConnect();
	return hsSerial->read();
} 

void EEGheadset::getData() {
	uint8_t const SYNC = 0xAA;
	uint8_t payloadLength = 0;
	uint8_t payloadData[0xFF] = {0};

//step 1
  if(readOneByte()==SYNC) { 
// step 2
    if(readOneByte()==SYNC) { 
// step 3
      payloadLength = readOneByte();
// step 3.1
      if(payloadLength>(SYNC-1)) {
// step 3.2
        if(payloadLength>SYNC) {
        	sprintf(lastError, MSG_ERRORS_PLENGTH_TOO_LARGE);
          	// return false;
        }
      } else {
// step 4
        generatedChecksum = 0;        
        for(uint8_t i=0;i<payloadLength;i++) {
          payloadData[i] = readOneByte();
          generatedChecksum += payloadData[i];
        } //for
// step 5
		//Read checksum byte from stream
        checksum = readOneByte();                     
		//Take one's compliment of generated checksum         
        generatedChecksum = 0xFF-generatedChecksum;   
// step 6
        if(checksum==generatedChecksum) {
          sprintf(lastError, MSG_ERRORS_NONE);
		  //Parse the payload
// step 6.1 
          for(byte i=0;i<payloadLength;i++) {
            switch (payloadData[i]) {
			  //POOR_SIGNAL Quality (0-0xFF)
              case 0x02:
                poorQuality = payloadData[++i];
                break;
			  //ATTENTION eSense (0 to 100)
              case 0x04:
                attention = payloadData[++i];
                break;
			  //MEDITATION eSense (0 to 100)
               case 0x05:
                meditation = payloadData[++i];
                break;
			  //Blink Strength. (0-255) Sent only when Blink event occurs.
              case 0x16:
             	blinkStrength = payloadData[++i];
                break;
              case 0x80:
			  // RAW Wave Value: a single big-endian 16-bit two's-compliment signed 
			  // value  
			  // (high-order byte followed by low-order byte)              
                i = i+3;
                break;
              case 0x83:
			  // ASIC_EEG_POWER: eight big-endian 3-byte unsigned integer values 
			  // representing delta, theta, low-alpha high-alpha, low-beta, 
			  // high-beta, low-gamma, and mid-gamma EEG band power values
				eegPowerLength = payloadData[++i];
			  // Extract the values.
				for(uint8_t j=0;j<8;j++) 
					eegPower[j] = ((uint32_t)payloadData[++i]<<16) 
								| ((uint32_t)payloadData[++i]<<8) 
								| (uint32_t)payloadData[++i];
                break;
              default:
                break;
            } // switch (payloadData[i])
          } // for
        } else sprintf(lastError, MSG_ERRORS_CHECKSUM_ERROR);
      } // end step 4
    } // end if read 2nd SYNC byte
  } // end if read 1st SYNC byte
}

/*
TODO: 
char* EEGheadset::serializeJSON() {
	return "";
}
*/

void EEGheadset::printDataToSerial() {
	Serial.println("-=- Start Packet -=-");
	Serial.print("Poor Quality: ");
	Serial.println(poorQuality, DEC);
	Serial.print("Attention: ");
	Serial.println(attention, DEC);
	Serial.print("Meditation: ");
	Serial.println(meditation, DEC);
	Serial.print("Blink Strength: ");
	Serial.println(blinkStrength, DEC);
	Serial.println("\tEEG POWER:");
	Serial.print("\t\tDelta: ");
	Serial.println(eegPower[0], DEC);
	Serial.print("\t\tTheta: ");
	Serial.println(eegPower[1], DEC);
	Serial.print("\t\tLow Alpha: ");
	Serial.println(eegPower[2], DEC);
	Serial.print("\t\tHigh Alpha: ");
	Serial.println(eegPower[3], DEC);
	Serial.print("\t\tLow Beta: ");
	Serial.println(eegPower[4], DEC);
	Serial.print("\t\tHigh Beta: ");
	Serial.println(eegPower[5], DEC);
	Serial.print("\t\tLow Gamma: ");
	Serial.println(eegPower[6], DEC);
	Serial.print("\t\tMid Gamma: ");
	Serial.println(eegPower[7], DEC);
	Serial.print("Checksum Calculated: ");
	Serial.println(generatedChecksum, HEX);
	Serial.print("Checksum Expected: ");
	Serial.println(checksum, HEX);
	Serial.println("-=- End Packet -=-\n");
}

char* EEGheadset::getError() {
	return lastError;
}

uint8_t EEGheadset::getPoorQuality() {
	return poorQuality;
}

uint8_t EEGheadset::getAttention() {
	return attention;
}

uint8_t EEGheadset::getMeditation() {
	return meditation;
}

uint8_t EEGheadset::getBlinkStrength() {
	return blinkStrength;
}

uint32_t* EEGheadset::getEEGPower() {
	return eegPower;
}

uint32_t EEGheadset::getDelta() {
	return eegPower[0];
}

uint32_t EEGheadset::getTheta() {
	return eegPower[1];
}

uint32_t EEGheadset::getLowAlpha() {
	return eegPower[2];
}

uint32_t EEGheadset::getHighAlpha() {
	return eegPower[3];
}

uint32_t EEGheadset::getLowBeta() {
	return eegPower[4];
}

uint32_t EEGheadset::getHighBeta() {
	return eegPower[5];
}

uint32_t EEGheadset::getLowGamma() {
	return eegPower[6];
}

uint32_t EEGheadset::getMidGamma() {
	return eegPower[7];
}

char* const EEGheadset::MSG_ERRORS_NONE = "Errors is absent";
char* const EEGheadset::MSG_ERRORS_HEADSET1 = "headset ID empty";
char* const EEGheadset::MSG_ERRORS_PLENGTH_TOO_LARGE = "Packet length is too large";
char* const EEGheadset::MSG_ERRORS_CHECKSUM_ERROR = "Checksum error";
char* const EEGheadset::MSG_ERRORS_TRY_CONNECT = "Try connect";
char* const EEGheadset::MSG_ERRORS_TRY_DISCONNECT = "Try disconnect";