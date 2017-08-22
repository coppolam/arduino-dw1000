/*
 * ConnectedRanging.cpp
 *
 *  Created on: Aug 18, 2017
 *      Author: steven <stevenhelm@live.nl>
 */


#include "ConnectedRanging.h"

ConnectedRangingClass ConnectedRanging;

// data buffer
byte ConnectedRangingClass::_data[MAX_LEN_DATA] = {2, 1, POLL, 3, RANGE, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 4, RANGE_REPORT, 1, 2, 3, 4, 5, POLL_ACK};

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

// nodes (_numNodes includes the current device, but _networkNodes does not)
DW1000Node ConnectedRangingClass::_networkNodes[MAX_NODES];
uint8_t ConnectedRangingClass::_numNodes = 0;

// when it is time to send
boolean ConnectedRangingClass::_timeToSend = false;




// initialization function
void ConnectedRangingClass::ConnectedRangingClass::init(char longAddress[], uint8_t numNodes){
	// nodes to range to
	if(numNodes<=MAX_NODES){
		_numNodes = numNodes;
	}
	else{
		Serial.println("The desired number of nodes exceeds MAX_NODES");
		_numNodes=MAX_NODES;
	}
	DW1000.convertToByte(longAddress, _longAddress);
	initDecawave(_longAddress, numNodes);
	initNodes();
	_lastSent = millis();

}

// initialization function
void ConnectedRangingClass::init(uint8_t veryShortAddress,uint8_t numNodes){
	// nodes to range to
	if(numNodes<=MAX_NODES){
		_numNodes = numNodes;
	}
	else{
		Serial.println("The desired number of nodes exceeds MAX_NODES");
		_numNodes=MAX_NODES;
	}
	for(int i=0;i<LEN_EUI;i++){
		_longAddress[i] = veryShortAddress;
	}
	initDecawave(_longAddress, numNodes);
	initNodes();
	_lastSent = millis();
	handleMessage();

}

// initialization function
void ConnectedRangingClass::initNodes(){
	byte address[LEN_EUI];
	uint8_t remoteShortAddress = 1;
	for (int i=0;i<_numNodes-1;i++){
		if(remoteShortAddress==_veryShortAddress){
			remoteShortAddress++;
		}
		if(remoteShortAddress!=_veryShortAddress){
			for (int j=0;j<LEN_EUI;j++){
				address[j]=remoteShortAddress;
			}
			DW1000Node temp = DW1000Node(address);
			_networkNodes[i]=temp;
			remoteShortAddress++;
		}

	}
}

// initialization function
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

void ConnectedRangingClass::handleMessage(){
	uint8_t messagefrom = _data[0];
	Serial.print("Message from: ");Serial.println(messagefrom);
	if (messagefrom==_veryShortAddress-1){
		Serial.println("It is time to send!");
		_timeToSend = true;
	}
	uint16_t datapointer = 0;
	uint8_t toDevice;
	for (int i=0; i<_numNodes-1;i++){
		datapointer++;
		toDevice = _data[datapointer];
		Serial.print("toDevice is: ");Serial.println(toDevice);
		if(toDevice!=_veryShortAddress){
			incrementDataPointer(&datapointer);
		}
		else{
			processMessage(messagefrom,&datapointer);
			incrementDataPointer(&datapointer);
		}

	}
}

// if this part of the message was not for the current device, skip the pointer ahead to next block of _data
void ConnectedRangingClass::incrementDataPointer(uint16_t *ptr){
	uint8_t msgtype = _data[*ptr+1];
	switch(msgtype){
		case POLL : *ptr += POLL_SIZE;
					break;
		case POLL_ACK : *ptr += POLL_ACK_SIZE;
					break;
		case RANGE : *ptr += RANGE_SIZE;
					break;
		case RANGE_REPORT : *ptr += RANGE_REPORT_SIZE;
					break;
	}

}

void ConnectedRangingClass::processMessage(uint8_t msgfrom, uint16_t *ptr){
	uint8_t nodeIndex = msgfrom -1 - (uint8_t)(_veryShortAddress<msgfrom);
	Serial.print(F("Message from: "));Serial.println(msgfrom);
	Serial.print(F("nodeIndex: "));Serial.println(nodeIndex);
	DW1000Node *distantNode = &_networkNodes[nodeIndex];
	uint8_t msgtype = _data[*ptr+1];
	if(msgtype == POLL){
		distantNode->setStatus(2);
		DW1000.getReceiveTimestamp(distantNode->timePollReceived);
		Serial.print(F("POLL processed for distantNode "));Serial.println(distantNode->getShortAddress(),HEX);
	}
	else if(msgtype == POLL_ACK){
		distantNode->setStatus(4);
		DW1000.getReceiveTimestamp(distantNode->timePollAckReceived);
		Serial.print(F("POLL ACK processed for distantNode "));Serial.println(distantNode->getShortAddress(),HEX);
	}
	else if(msgtype == RANGE){
		distantNode->setStatus(6);
		DW1000.getReceiveTimestamp(distantNode->timeRangeReceived);
		Serial.print(F("RANGE processed for distantNode "));Serial.println(distantNode->getShortAddress(),HEX);
	}
	else if(msgtype == RANGE_REPORT){
		distantNode->setStatus(0);
		Serial.print(F("RANGE_REPORT processed for distantNode "));Serial.println(distantNode->getShortAddress(),HEX);
	}
}

