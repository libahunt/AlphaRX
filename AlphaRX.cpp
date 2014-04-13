/*******************************************************************************
AlphaRX.cpp - An Arduino library for RFSolutions FM receiver module.
		Datasheet: http://www.rfsolutions.co.uk/acatalog/DS-ALPHATX-3.pdf
		Si4320 (used inside AlphaRX module) datasheet - is more helpful:
		http://www.silabs.com/Support%20Documents/TechnicalDocs/Si4320.pdf

By Anna Jõgi a.k.a Libahunt Sept 25 2013. Version 0.1


*******************************************************************************/

#include "Arduino.h"
#include "AlphaRX.h"

AlphaRX::AlphaRX(byte rxnsel, byte rxsdo, byte rxsdi, byte rxsck, byte rxnirq, byte rxnffs) {

  _rxnsel = rxnsel;
  _rxsdo = rxsdo;
	_rxsdi = rxsdi;
	_rxsck = rxsck;
	_rxnirq = rxnirq;
	_rxnffs = rxnffs;

  pinMode(_rxnsel, OUTPUT);
	pinMode(_rxsdo, OUTPUT);
	pinMode(_rxsdi, INPUT);
  pinMode(_rxsck, OUTPUT);
  pinMode(_rxnirq, INPUT);
  pinMode(_rxnffs, OUTPUT);

	digitalWrite(_rxnirq, 1); //enable internal pullup
	digitalWrite(_rxsdo, 1); //enable internal pullup

}
 
/*** PUBLIC FUNCTIONS *********************************************************/


void AlphaRX::initRXDefault() {

//initializes AlphaRX module with default settings

  sendCmd(B10001001, B00111011);
  sendCmd(B10100110, B00100000);
  sendCmd(B11000000, B11000011);
  sendCmd(B11001100, B00001110);
  sendCmd(B11000110, B11110111);
  sendCmd(B11000100, B11101100);
  sendCmd(B11001000, B00100011);
  
  sendCmd(B00000000, B00000000);
  
  delayMicroseconds(2);
  sendCmd(B11001110, B11111000);
  delayMicroseconds(2);
  sendCmd(B11001110, B11111011);
  delayMicroseconds(2);
  sendCmd(B11000000, B11000011);

}

void AlphaRX::sendCmd(byte command1, byte command2) {

//writes a single command to AlphaRX module - see datasheet and example sketch

  byte bitmask;
  digitalWrite(_rxnsel, 0);
  delayMicroseconds(1);
  for (bitmask = B10000000; bitmask > 0; bitmask = bitmask/2) {
    if (command1 & bitmask) {
      cmdOut(1);
    }
    else {
      cmdOut(0);
    }
  }
  for (bitmask = B10000000; bitmask > 0; bitmask = bitmask/2) {
    if (command2 & bitmask) {
      cmdOut(1);
    }
    else {
      cmdOut(0);
    }
  }
  digitalWrite(_rxnsel, 1);
	digitalWrite(_rxsdo, 0);
	digitalWrite(_rxsck, 0);
  delayMicroseconds(1);
}



byte AlphaRX::getDataPacket(unsigned long waittime, byte *RFData) {

//fills passed RFData array: RFData[0] - label byte (uses 4 bits as B0000xxxx), RFData[1] - value byte
//returns 	0 if no transmission was received e.g timeout
//					1 if checksum verifies
//					2 in case of invalid checksum e.g data received faulty

	//clear old data
	RFData[0] = 0;
	RFData[1] = 0;

	digitalWrite(_rxnsel, 1);

	unsigned long starttime = millis();
	while (digitalRead(_rxnirq)) {
		//loops until RX module's nirq signals data coming in or timeout
		if (millis() >= starttime + waittime) {//no data has came in during waittime
			return 0;
		}
	}

	//some data came in during waittime, read it
	byte inByte1 = getByte();
	byte inByte2 = getByte();

	//first byte contains checksum as 4 MSB bits
	byte receivedChecksum = inByte1 >> 4;
	
	//first byte contains (optional) label as 4 LSB bits
	byte label = inByte1 & B00001111;

	RFData[0] = label;
	RFData[1] = inByte2;

	//calculate checksum of received value and label
	byte valuePart1 = inByte2 >> 4;
	byte valuePart2 = inByte2 & B00001111;
	byte localChecksum = valuePart1 ^ valuePart2;
	localChecksum = localChecksum ^ label;

	//test checksum match
	if (localChecksum == receivedChecksum) {//checksums match, data is valid
		return 1;
	}
	else {//checksums do not match, data scattered
		return 2;
	}
	
}



void AlphaRX::getStatus(byte *alphaRXStatus) {

//fills passed array with two bytes of status - See "12. Status read command" in Si4320 datasheet

//Meaning of status value bits:
//alphaRXStatus[0]
//  MSB 7 	Number of the data bits in the FIFO is reached the preprogrammed limit
//			6		FIFO overflow
//			5		Wake-up timer overflow
//			4		Low battery detect, voltage is below preprogrammed limit
//			3		FIFO is empty
//			2		Strangth of the incoming signal is above the preprogrammed limit
//			1		Data Quality Detector detected a good quality signal
//	LSB	0		Clock recovery lock
//alphaRXStatus[1]
//  MSB 7 	Toggling in each AFC cycle
//			6		AFC stabilized (measured twice the same offset value)
//			5		Sign of offset value to be added to the value of the Frequency control word
//			4-0 Offset value to be added to the value of the Frequency control word

	digitalWrite(_rxnsel, 0);
  delayMicroseconds(1);
  digitalWrite(_rxsdo, 0);//Alpha module will undestand first 0 bit (on SCK rising edge?) as status read command and has FIFO IT bit already on it's SDO
	delayMicroseconds(1);

	alphaRXStatus[0] = 0;
	alphaRXStatus[1] = 0;
	int i;
	byte inBit;
	for (i = 0; i < 8; i++) {
		digitalWrite(_rxsck, 1); //rising edge to read a bit
		delayMicroseconds(1);
		inBit = digitalRead(_rxsdi);
		alphaRXStatus[0] = alphaRXStatus[0] * 2 + inBit;
		digitalWrite(_rxsck, 0); //falling edge
		delayMicroseconds(1);
	}
	for (i = 0; i < 8; i++) {
		digitalWrite(_rxsck, 1); //rising edge to read a bit
		delayMicroseconds(1);
		inBit = digitalRead(_rxsdi);
		alphaRXStatus[1] = alphaRXStatus[1] * 2 + inBit;
		digitalWrite(_rxsck, 0); //falling edge
		delayMicroseconds(1);
	}

  digitalWrite(_rxnsel, 1);
	digitalWrite(_rxsdo, 0);

}


/*** PRIVATE FUNCTIONS ********************************************************/

void AlphaRX::cmdOut(boolean sendbit) {
  digitalWrite(_rxsdo, sendbit);
	delayMicroseconds(1);
  digitalWrite(_rxsck, 1); //rising edge for AlphaRX to read bit
  delayMicroseconds(1);
  digitalWrite(_rxsck, 0); //falling edge
}


byte AlphaRX::getByte() {
	delayMicroseconds(1);
	digitalWrite(_rxnffs, 0);
  byte readByte = 0;
	int i;
	boolean inBit;
  for (i=0; i<8; i++) {
    digitalWrite(_rxsck, 1); //rising edge, max 2,5MHz clocking
		delayMicroseconds(1);
    inBit = digitalRead(_rxsdi);
    readByte = readByte * 2 + inBit;
    digitalWrite(_rxsck, 0); //falling edge
		delayMicroseconds(1);
  }
	digitalWrite(_rxnffs, 1);
  return readByte;
}


