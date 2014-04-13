/*******************************************************************************
AlphaRX.h - An Arduino library for RFSolutions FM receiver module.
		Datasheet: http://www.rfsolutions.co.uk/acatalog/DS-ALPHATX-3.pdf
		Si4320 (used inside AlphaRX module) datasheet - is more helpful:
		http://www.silabs.com/Support%20Documents/TechnicalDocs/Si4320.pdf

By Anna Jõgi a.k.a Libahunt Sept 25 2013. Version 0.1


*******************************************************************************/

#ifndef ALPHA_RX_H
#define ALPHA_RX_H

#include "Arduino.h"

class AlphaRX {
 
	public:
		
		AlphaRX(byte rxnsel, byte rxsdo, byte rxsdi, byte rxsck, byte rxnirq, byte rxnffs);

		void sendCmd(byte command1, byte command2);
		void initRXDefault();
		byte getDataPacket(unsigned long waittime, byte *RFData);
		void getStatus(byte *alphaRXStatus);
		
	 
	private:
		
		byte _rxnsel; //PB6
		byte _rxsdo; //PB4
		byte _rxsdi; //PH4
		byte _rxsck; //PB5
		byte _rxnirq; //PH5
		byte _rxnffs; //PH&
		
		
		byte getByte();
		void cmdOut(boolean sendbit);
 
};
 
#endif
