/*
 * ConnectedRanging.cpp
 *
 *  Created on: Aug 18, 2017
 *      Author: steven
 */


#include "ConnectedRanging.h"

ConnectedRangingClass ConnectedRanging;

uint8_t ConnectedRangingClass::_RST;
uint8_t ConnectedRangingClass::_SS;
uint8_t ConnectedRangingClass::_IRQ;
byte ConnectedRangingClass::_longAddress[LEN_EUI];
byte ConnectedRangingClass::_veryShortAddress;



void ConnectedRangingClass::init(char longAddress[], uint16_t networkID,const byte mode[],uint8_t myRST, uint8_t mySS, uint8_t myIRQ){
	_RST = myRST;
	_SS = mySS;
	_IRQ = myIRQ;
	DW1000.begin(myIRQ,myRST);
	DW1000.select(mySS);
	//Save the address
	DW1000.convertToByte(longAddress, _longAddress);
	_veryShortAddress = _longAddress[0];
	//Write the address on the DW1000 chip
	DW1000.setEUI(_longAddress);
	Serial.print("device address: ");
	Serial.println(longAddress);
	Serial.print("very short device address: ");
	Serial.println(_veryShortAddress);

	//Setup DW1000 configuration
	DW1000.newConfiguration();
	DW1000.setDefaults();
	DW1000.setDeviceAddress((uint16_t)_veryShortAddress);
	DW1000.setNetworkId(networkID);
	DW1000.enableMode(mode);
	DW1000.commitConfiguration();
}



