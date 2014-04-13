/******************************************************************************
Example sketch for AlphaRX library.

Use AlphaTX module with it's library's example sketch to send data.
Connect AlphaRX module to serial port to see results.

Insert any letter in serial monitor and press enter then after 
current receiving sequence ends the status of AlphaRX is printed out.

This library uses FIFO mode that can only receive two bytes of data at a time.
To also include checksum in the data packet, the first byte can only have 
four bits of information in it - that is decimal value 0-15 or hexadecimal 0-f.
In AlphaTX example this part of data is called a "label". The other part 
is a full byte and called "value" in AlphaTX sketch.

Known issue with this sketch - sometimes datapackets containing 0 0 value
are erroneously read out. When using the library do not send 0 0 and discard
the packet if this value is received.
*******************************************************************************/

#include <AlphaRX.h>

// Arduino pin number     Alpha module pin number and name
byte rxsdi  = 7; //        1 FFIT / SDO
byte rxnirq = 8; //        2 nIRQ
byte rxnffs = 9; //	   3 DATA / nFFS
byte rxsdo = 10; //        12 SDI
byte rxsck = 11; //        13 SCK
byte rxnsel = 12;//        14 nSEL

//A global two element byte array is needed for AlphaRX library to work, to be passed to alphaRX.getDataPacket() function
byte RFdata[2] = {0, 0};

unsigned long waittime = 10000; //specifies how long should a transmission start be waited for, in milliseconds, to be passed to alphaRX.getDataPacket() function

//This global two element byte array is needed for troubleshooting purposes only, to be passed to alphaRX.getStatus() function
byte alphaRXStatus[2] = {0, 0};




//Create an instance of the AlphaRX class
AlphaRX alphaRX = AlphaRX(rxnsel, rxsdo, rxsdi, rxsck, rxnirq, rxnffs);




void setup() {
 
  
  //pinModes for AlphaRX module were automatically declared when creating an instance
  
  Serial.begin(9600);
  
  
/*----------------------------------------------------------------------------------------------
1. AlphaRX module has to be initialized to right frequency and settings
----------------------------------------------------------------------------------------------*/
  
/*** Initializing AlphaRX module ****************************************************************
Modify following commands or add needed ones according to AlphaRX module datasheet instructions 
***                                        Command name in datasheet                        ***/
  alphaRX.sendCmd(B10001001, B00111011); //Configuration setting
  alphaRX.sendCmd(B10100110, B00100000); //Frequency setting
  alphaRX.sendCmd(B11000000, B11000011); //Receiver setting
  alphaRX.sendCmd(B11001100, B00001110); //Low dudy cycle
  alphaRX.sendCmd(B11000110, B11110111); //AFC
  alphaRX.sendCmd(B11000100, B11101100); //Data filter
  alphaRX.sendCmd(B11001000, B00100011); //Data rate
  alphaRX.sendCmd(B00000000, B00000000); //Status read
  delayMicroseconds(2);
  alphaRX.sendCmd(B11001110, B11111000); //Output and FIFO mode
  delayMicroseconds(2);
  alphaRX.sendCmd(B11001110, B11111011); //Output and FIFO mode
  delayMicroseconds(2);
  alphaRX.sendCmd(B11000000, B11000011); //Receiver setting
/*end of initializing commands*/
  
/*Alternatively, if you don't need to change frequency or other settings, you can use a short default initialzation function:*/
  //alphaRX.initRXDefault();
  
  
  
} //end setup()



void loop() {

/*----------------------------------------------------------------------------------------------
2. Trying to receive a transmission. Then doing stuff with it, like printing results to serial.
----------------------------------------------------------------------------------------------*/
 
  Serial.print("Waiting transmission ... ");  
	
	
  //alphaRX.getDataPacket function fills RFdata array with received data and returns success status as number
  byte getDataPacketStatus = alphaRX.getDataPacket(waittime, RFdata);
  

  Serial.print(" ");
  if (getDataPacketStatus == 2) {
    Serial.println("                                                      Failed.");
    Serial.print("Transmission received, but data is scattered. Received data (in HEX): ");
    int i;
    for (i=0; i<2; i++) {
      Serial.print(String(RFdata[i], HEX));
      Serial.print(" ");
    }
  }
  else if (getDataPacketStatus == 1) {
    Serial.println("Yay! Transmission received, checksum is right!");
    Serial.print("                                                         Received data (in HEX): ");
    int i;
    for (i=0; i<2; i++) {
      Serial.print(String(RFdata[i], HEX));
      Serial.print(" ");
    }
  }
  else if (getDataPacketStatus == 0) {
    Serial.print("                                         Timeout, no transmission.");
  }
  Serial.println();
  
  
  
  
/*----------------------------------------------------------------------------------------------
2.B Example troubleshooting sequence - will print status info if you enter anything in serial montor
----------------------------------------------------------------------------------------------*/
  
  if (Serial.available() > 0) {

    Serial.read();//clears serial input

    alphaRX.getStatus(alphaRXStatus); //gets AlphaRX status reading and stores it two element byte array that has to be passed to the function
    printAlphaRXStatus(alphaRXStatus); //prints to serial monitor status values from the passed array and their meaning

  }
 
    

 
  
/*----------------------------------------------------------------------------------------------
3. Re-initialize AlphaRX module to receive next data packet
----------------------------------------------------------------------------------------------*/
  alphaRX.sendCmd(B00000000, B00000000); //Status read
  delayMicroseconds(2);
  alphaRX.sendCmd(B11001110, B11111000); //Output and FIFO mode
  delayMicroseconds(2);
  alphaRX.sendCmd(B11001110, B11111011); //Output and FIFO mode
  delayMicroseconds(2);
  alphaRX.sendCmd(B11000000, B11000011); //Receiver setting

  
} //end loop()






/*Bonus: troubleshooting made easy.
If having trouble copy this function to your sketch and use it to easily check status.

!!! alphaRX.getStatus() function has to be called before calling this function!

Note that printing to serial is very time consuming therefore in your code you may need to 
call alphaRX.getStatus()and printAlphaRXStatus() in separate places from each other. 
Status values are kept until new call is made to alphaRX.getStatus().

As argument pass the same array that was passed to alphaRX.getStatus(). */ 

void printAlphaRXStatus(byte statusArray[2]) {
  
  Serial.println();
  Serial.println();
  Serial.print("AlphaRX status: ");
  Serial.print(statusArray[0], BIN);
  Serial.print(" ");
  Serial.println(statusArray[1], BIN);
  Serial.println();

  if (statusArray[0] & B10000000) {Serial.print(1);} else {Serial.print(0);}    
  Serial.println(" FIFO IT - Number of the data bits in the FIFO is reached the preprogrammed limit");
  if (statusArray[0] & B01000000) {Serial.print(1);} else {Serial.print(0);}     
  Serial.println(" FFOV - FIFO overflow");
  if (statusArray[0] & B00100000) {Serial.print(1);} else {Serial.print(0);}     
  Serial.println(" WK-UP - Wake-up timer overflow");
  if (statusArray[0] & B00010000) {Serial.print(1);} else {Serial.print(0);}     
  Serial.println(" LBD - Low battery detect, the power supply voltage is below the preprogrammed limit");
  if (statusArray[0] & B00001000) {Serial.print(1);} else {Serial.print(0);}     
  Serial.println(" FFEM - FIFO is empty");
  if (statusArray[0] & B00000100) {Serial.print(1);} else {Serial.print(0);}    
  Serial.println(" DRSSI - The strength of the incoming signal is above the preprogrammed limit");
  if (statusArray[0] & B00000010) {Serial.print(1);} else {Serial.print(0);}     
  Serial.println(" DQD - Data Quality Detector detected a good quality signal");
  if (statusArray[0] & B00000001) {Serial.print(1);} else {Serial.print(0);}     
  Serial.println(" CRL - Clock recovery lock");
  
  if (statusArray[1] & B10000000) {Serial.print(1);} else {Serial.print(0);}    
  Serial.println(" ATGL - Toggling in each AFC cycle");
  if (statusArray[1] & B01000000) {Serial.print(1);} else {Serial.print(0);}     
  Serial.println(" ASAME - AFC stabilized (measured twice the same offset value)");
  Serial.print("Offset value to be added to the value of the Frequency control word*:  ");
  if (statusArray[1] & B00100000) {Serial.print("-");} else {Serial.print("+");} 
  Serial.print((statusArray[1] & B00011111), BIN);
  Serial.println(", BIN");
  Serial.println("* See Si4320 datasheet, pages 15 and 22");
  Serial.println();
  Serial.println();
  
}


