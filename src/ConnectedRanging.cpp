/*
 * ConnectedRanging.cpp
 *
 *  Created on: Aug 18, 2017
 *      Author: steven
 */


#include "ConnectedRanging.h"

ConnectedRangingClass ConnectedRanging;

// data buffer
byte ConnectedRangingClass::_data[MAX_LEN_DATA];

// pins on the arduino used to communicate with DW1000
uint8_t ConnectedRangingClass::_RST;
uint8_t ConnectedRangingClass::_SS;
uint8_t ConnectedRangingClass::_IRQ;

// addresses of current DW1000
byte ConnectedRangingClass::_longAddress[LEN_EUI];
byte ConnectedRangingClass::_veryShortAddress;

// message sent/received state
volatile boolean ConnectedRangingClass::_sentAck     = false;
volatile boolean ConnectedRangingClass::_receivedAck = false;

// keeping track of send times
uint32_t ConnectedRangingClass::_lastSent = 0;

// nodes
DW1000Node ConnectedRangingClass::_networkNodes[MAX_NODES];
uint8_t ConnectedRangingClass::_numNodes = 0;





void ConnectedRangingClass::ConnectedRangingClass::init(char longAddress[], uint8_t numNodes){
	// nodes to range to
	_numNodes = numNodes;
	initNodes();
	DW1000.convertToByte(longAddress, _longAddress);
	initDecawave(_longAddress, numNodes);

}

void ConnectedRangingClass::init(uint8_t veryShortAddress,uint8_t numNodes){
	// nodes to range to
	_numNodes = _numNodes;
	initNodes();
	for(int i=0;i<LEN_EUI;i++){
		_longAddress[i] = veryShortAddress;
	}
	initDecawave(_longAddress, numNodes);

}

void ConnectedRangingClass::initNodes(){
	byte address[LEN_EUI];
	uint8_t remoteShortAddress = 1;
	for (int i=0;i<_numNodes;i++){
		if(remoteShortAddress!=_veryShortAddress){
			for (int j=0;j<LEN_EUI;j++){
				address[j]=remoteShortAddress;
			}
			DW1000Node temp = DW1000Node(address);
			_networkNodes[i]=temp;

		}
		remoteShortAddress++;
	}
}

void ConnectedRangingClass::initDecawave(byte longAddress[], uint8_t numNodes, const byte mode[], uint16_t networkID,uint8_t myRST, uint8_t mySS, uint8_t myIRQ){
	_RST = myRST;
	_SS = mySS;
	_IRQ = myIRQ;
	DW1000.begin(myIRQ,myRST);
	DW1000.select(mySS);
	//Save the address
	_veryShortAddress = _longAddress[0];
	//Write the address on the DW1000 chip
	DW1000.setEUI(_longAddress);
	Serial.print("very short device address: ");
	Serial.println(_veryShortAddress);

	//Setup DW1000 configuration
	DW1000.newConfiguration();
	DW1000.setDefaults(true);
	DW1000.setDeviceAddress((uint16_t)_veryShortAddress);
	DW1000.setNetworkId(networkID);
	DW1000.enableMode(mode);
	DW1000.commitConfiguration();
	DW1000.attachSentHandler(handleSent);
	DW1000.attachReceivedHandler(handleReceived);
	receiver();
	_lastSent = millis();
}



void ConnectedRangingClass::loopReceive(){
	if(_receivedAck){
		_receivedAck = false;
		Serial.print("message received: ");
		DW1000.getData(_data,MAX_LEN_DATA);
		char msg[MAX_LEN_DATA];
		DW1000.convertBytesToChars(_data,msg,MAX_LEN_DATA);
		/*
		for(int i=0;i<MAX_LEN_DATA;i++){
			Serial.println(_data[i]);
		}*/
		Serial.println(msg);
	}

}

void ConnectedRangingClass::loopTransmit(char msg[],uint16_t n){
	if(_sentAck){
		_sentAck = false;
		Serial.print("Sent message: ");
		Serial.println(msg);
	}
	if (millis()-_lastSent > transmitDelay){
		_lastSent = millis();
		transmitData(msg,n);
	}

}

void ConnectedRangingClass::loopTransmit(){
	if(_sentAck){
		_sentAck = false;
		Serial.println("message sent!");
	}
	if (millis()-_lastSent > transmitDelay){
		_lastSent = millis();
		_data[0]=68;
		transmitData(_data);
	}

}



void ConnectedRangingClass::receiver() {
	DW1000.newReceive();
	DW1000.setDefaults(true);
	// so we don't need to restart the receiver manually
	DW1000.receivePermanently(true);
	DW1000.startReceive();
}

void ConnectedRangingClass::transmitData(byte datas[]){
	DW1000.newReceive();
	DW1000.setDefaults(true);
	DW1000.setData(datas,MAX_LEN_DATA);
	DW1000.startTransmit();
}

void ConnectedRangingClass::transmitData(char datas[]){
	DW1000.convertCharsToBytes(datas, _data,MAX_LEN_DATA);
	DW1000.newReceive();
	DW1000.setDefaults(true);
	DW1000.setData(_data,MAX_LEN_DATA);
	DW1000.startTransmit();
}

void ConnectedRangingClass::transmitData(char datas[],uint16_t n){
	DW1000.convertCharsToBytes(datas, _data,n);
	DW1000.newReceive();
	DW1000.setDefaults(true);
	DW1000.setData(_data,n);
	DW1000.startTransmit();
}


void ConnectedRangingClass::handleSent() {
	// status change on sent success
	_sentAck = true;
}

void ConnectedRangingClass::handleReceived() {
	// status change on received success
	_receivedAck = true;
}


