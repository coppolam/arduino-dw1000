/*
 * ConnectedRanging.cpp
 *
 *  Created on: Aug 18, 2017
 *      Author: steven <stevenhelm@live.nl>
 */


#include "ConnectedRanging.h"

ConnectedRangingClass ConnectedRanging;

// data buffer. Format of data buffer is like this: {from_address, to_address_1, message_type, additional_data, to_address_2, message_type, additional_data,.... to_address_n, message_type, additional data}
// Every element takes up exactly 1 byte, except for additional_data, which takes up 15 bytes in the case of a RANGE message, and 4 bytes in the case of RANGE_REPORT
// Example: byte _data[] = {2, 1, POLL, 3, RANGE, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 4, RANGE_REPORT, 1, 2, 3, 4, 5, POLL_ACK};
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

// nodes (_numNodes includes the current device, but _networkNodes does not)
DW1000Node ConnectedRangingClass::_networkNodes[MAX_NODES];
uint8_t ConnectedRangingClass::_numNodes = 0;

// when it is time to send
boolean ConnectedRangingClass::_timeToSend = false;

// remembering future time in case RANGE message is sent
boolean ConnectedRangingClass::_rangeSent = false;
DW1000Time ConnectedRangingClass::_rangeTime;

// extended frame
boolean ConnectedRangingClass::_extendedFrame = false;

uint16_t ConnectedRangingClass::protTimes = 0;

// last noted activity
uint32_t ConnectedRangingClass::_lastActivity;



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
	_lastActivity = millis();


	Serial.print(F("Initialization complete, this device's Short Address is: "));
	Serial.print(_veryShortAddress,HEX);
	Serial.print(F(" Long Address: "));

	for(int i=0; i<LEN_EUI;i++){
		Serial.print(_longAddress[i]);
	}
	Serial.println(" ");
	Serial.print("Extended frame is: ");
	Serial.println(_extendedFrame);
	Serial.print(F("The distant devices in memory are: "));
	for(int i=0; i<_numNodes-1;i++){
		_networkNodes[i].printNode();
	}
/*
	// Trying out range computation with DW1000Time
	DW1000Time pollsent = DW1000Time(459063425024);
	DW1000Time pollreceived = DW1000Time(264235075712);
	DW1000Time pollacksent = DW1000Time(269000431616);
	DW1000Time pollackreceived = DW1000Time(463828790040);
	DW1000Time rangesent = DW1000Time(466395586560);
	DW1000Time rangereceived = DW1000Time(271259151599);

	DW1000Time round1 = pollackreceived-pollsent;
	DW1000Time reply1 = pollacksent-pollreceived;
	DW1000Time round2 = rangereceived-pollacksent;
	DW1000Time reply2 = rangesent-pollackreceived;

	DW1000Time TOF = (round1*round2-reply1*reply2)/(round1+round2+reply1+reply2);

	Serial.print("Round 1 is: ");
	round1.print();
	//Serial.print("Round 1 printed is: "); Serial.println((long)round1.getTimestamp());
	Serial.print("Reply 1 is: ");
	reply1.print();
	Serial.print("Round 2 is: ");
	round2.print();
	//Serial.print("Round 2 printed is: "); Serial.println((long)round2.getTimestamp());
	Serial.print("Reply 2 is: ");
	reply2.print();

	Serial.print("TOF is: ");
	TOF.print();
	Serial.print("TOF in meters is: ");
	Serial.println(TOF.getAsMeters());


	DW1000Time round1wrap = round1.wrap();
	DW1000Time reply1wrap = reply1.wrap();
	DW1000Time round2wrap = round2.wrap();
	DW1000Time reply2wrap = reply2.wrap();

	DW1000Time TOFwrap = (round1wrap*round2wrap-reply1wrap*reply2wrap)/(round1wrap+round2wrap+reply1wrap+reply2wrap);



	Serial.print("Round 1 wrap is: ");
	round1wrap.print();
	Serial.print("Reply 1 wrap is: ");
	reply1wrap.print();
	Serial.print("Round 2 wrap is: ");
	round2wrap.print();
	Serial.print("Reply 2 wrap is: ");
	reply2wrap.print();

	Serial.print("TOFwrap is: ");
	TOFwrap.print();
	Serial.print("TOF in meters is: ");
	Serial.println(TOFwrap.getAsMeters());




*/


/*
	DW1000Time round1 = (myDistantDevice->timePollAckReceived-myDistantDevice->timePollSent).wrap();
		DW1000Time reply1 = (myDistantDevice->timePollAckSent-myDistantDevice->timePollReceived).wrap();
		DW1000Time round2 = (myDistantDevice->timeRangeReceived-myDistantDevice->timePollAckSent).wrap();
		DW1000Time reply2 = (myDistantDevice->timeRangeSent-myDistantDevice->timePollAckReceived).wrap();

		myTOF->setTimestamp((round1*round2-reply1*reply2)/(round1+round2+reply1+reply2));

*/

}

// initialization function
void ConnectedRangingClass::initNodes(){
	byte address[LEN_EUI];
	byte shortaddress[2];
	uint8_t remoteShortAddress = 1;
	for (int i=0;i<_numNodes-1;i++){
		if(remoteShortAddress==_veryShortAddress){
			remoteShortAddress++;
		}
		if(remoteShortAddress!=_veryShortAddress){
			for (int j=0;j<LEN_EUI;j++){
				address[j]=remoteShortAddress;
			}
			for (int j=0;j<2;j++){
				shortaddress[j]=remoteShortAddress;
			}
			DW1000Node temp = DW1000Node(address,shortaddress);
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
	Serial.print(F("very short device address: ")); Serial.println(_veryShortAddress);


	//Setup DW1000 configuration
	DW1000.newConfiguration();
	DW1000.setDefaults(_extendedFrame);
	DW1000.setDeviceAddress((uint16_t)_veryShortAddress);
	DW1000.setNetworkId(networkID);
	DW1000.enableMode(mode);
	DW1000.commitConfiguration();
	DW1000.attachSentHandler(handleSent);
	DW1000.attachReceivedHandler(handleReceived);
	receiver();

}

void ConnectedRangingClass::loop(){
	checkForReset();
	if(_sentAck){
		noteActivity();
		_sentAck = false;
		if(DEBUG){
			Serial.println("Sent a message:");
		}
		updateSentTimes();
		if(DEBUG){
			for(int i=0; i<MAX_LEN_DATA;i++){
				Serial.print(_data[i]);Serial.print(" ");
			}
			Serial.println(" ");
		}
	}
	if(_receivedAck){
		noteActivity();
		_receivedAck = false;
		//we read the datas from the modules:
		// get message and parse
		DW1000.getData(_data, MAX_LEN_DATA);
		handleReceivedData();
	}
	if (_veryShortAddress==1 && millis()-_lastSent>DEFAULT_RESET_TIME){
		_lastSent = millis();
		_timeToSend = true;
	}
	if(_timeToSend){
		_timeToSend = false;
		transmitInit();
		produceMessage();
		if(_rangeSent){
			_rangeSent = false;
			transmitData(_data);
		}
		else{
			DW1000Time delay = DW1000Time(DEFAULT_REPLY_DELAY_TIME, DW1000Time::MICROSECONDS);
			transmitData(_data,delay);
		}
	}

}

void ConnectedRangingClass::receiver() {
	DW1000.newReceive();
	DW1000.setDefaults(_extendedFrame);
	// so we don't need to restart the receiver manually
	DW1000.receivePermanently(true);
	DW1000.startReceive();
}

void ConnectedRangingClass::transmitInit(){
	DW1000.newTransmit();
	DW1000.setDefaults(_extendedFrame);
}

void ConnectedRangingClass::transmitData(byte datas[]){
	DW1000.setData(datas,MAX_LEN_DATA);
	DW1000.startTransmit();
}

void ConnectedRangingClass::transmitData(byte datas[], DW1000Time timeDelay){
	DW1000.setDelay(timeDelay);
	DW1000.setData(datas,MAX_LEN_DATA);
	DW1000.startTransmit();
}

void ConnectedRangingClass::transmitData(char datas[]){
	DW1000.convertCharsToBytes(datas, _data,MAX_LEN_DATA);
	DW1000.setData(_data,MAX_LEN_DATA);
	DW1000.startTransmit();
}

void ConnectedRangingClass::transmitData(char datas[],uint16_t n){
	DW1000.convertCharsToBytes(datas, _data,n);
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

void ConnectedRangingClass::handleReceivedData(){
	uint8_t messagefrom = _data[0];
	if(DEBUG){
		Serial.print("Message from: ");Serial.println(messagefrom);
	}
	if (messagefrom==_veryShortAddress-1){
		if(DEBUG){
			Serial.println("It is time to send!");
		}
		_timeToSend = true;
	}
	uint16_t datapointer = 1;
	uint8_t toDevice;
	for (int i=0; i<_numNodes-1;i++){
		toDevice = _data[datapointer];
		if(toDevice!=_veryShortAddress){
			incrementDataPointer(&datapointer);
		}
		else if(toDevice==_veryShortAddress){
			processMessage(messagefrom,&datapointer);
		}

	}
}

// if this part of the message was not for the current device, skip the pointer ahead to next block of _data
void ConnectedRangingClass::incrementDataPointer(uint16_t *ptr){
	uint8_t msgtype = _data[*ptr+1];
	switch(msgtype){
		case POLL : *ptr += POLL_SIZE+1;
					break;
		case POLL_ACK : *ptr += POLL_ACK_SIZE+1;
					break;
		case RANGE : *ptr += RANGE_SIZE+1;
					break;
		case RANGE_REPORT : *ptr += RANGE_REPORT_SIZE+1;
					break;
		case RECEIVE_FAILED : *ptr += RECEIVE_FAILED_SIZE+1;
					break;
	}

}

void ConnectedRangingClass::processMessage(uint8_t msgfrom, uint16_t *ptr){
	uint8_t nodeIndex = msgfrom -1 - (uint8_t)(_veryShortAddress<msgfrom);
	DW1000Node *distantNode = &_networkNodes[nodeIndex];
	uint8_t msgtype = _data[*ptr+1];
	*ptr+=2;
	if(msgtype == POLL){
		distantNode->setStatus(2);
		DW1000.getReceiveTimestamp(distantNode->timePollReceived);
		if(DEBUG){
			Serial.print(F("POLL message received from: ")); Serial.print(distantNode->getShortAddress(),HEX); Serial.print(F(" with timestamp: "));
			distantNode->timePollReceived.print(); Serial.print(F(" / ")); Serial.println(distantNode->timePollReceived.getAsMicroSeconds()/1000000,15);
		}
	}
	else if(msgtype == POLL_ACK){
		distantNode->setStatus(4);
		DW1000.getReceiveTimestamp(distantNode->timePollAckReceived);
		if(DEBUG){
			Serial.print(F("POLL ACK message received from: ")); Serial.print(distantNode->getShortAddress(),HEX); Serial.print(F(" with timestamp: "));
			distantNode->timePollAckReceived.print(); Serial.print(F(" / ")); Serial.println(distantNode->timePollAckReceived.getAsMicroSeconds()/1000000,15);
		}
	}
	// ADD THE ADDITIONAL INFO THAT A RANGE MESSAGE CONTAINS, TIME POLL SENT AND TIME RANGE SENT ARE UNKNOWN!
	else if(msgtype == RANGE){
		distantNode->setStatus(6);
		DW1000.getReceiveTimestamp(distantNode->timeRangeReceived);
		distantNode->timePollSent.setTimestamp(_data+*ptr);
		*ptr += 5;
		distantNode->timePollAckReceived.setTimestamp(_data+*ptr);
		*ptr += 5;
		distantNode->timeRangeSent.setTimestamp(_data+*ptr);
		*ptr += 5;
		DW1000Time TOF;
		computeRangeAsymmetric(distantNode, &TOF);
		float distance = TOF.getAsMeters();
		distantNode->setRange(distance);
		if(DEBUG){
			Serial.print(F("RANGE message received from: ")); Serial.print(distantNode->getShortAddress(),HEX); Serial.print(F(" with timestamp: "));
			distantNode->timeRangeReceived.print(); Serial.print(F(" /  "));
			Serial.println(distantNode->timeRangeReceived.getAsMicroSeconds()/1000000,15);
			/*
			Serial.println(F("Time stamps in range message:"));
			Serial.print(F("Time POLL sent: ")); distantNode->timePollSent.print(); Serial.print(F(" / ")); Serial.println(distantNode->timePollSent.getAsMicroSeconds()/1000000,15);
			Serial.print(F("Time POLL ACK received: ")); distantNode->timePollAckReceived.print(); Serial.print(F(" / ")); Serial.println(distantNode->timePollAckReceived.getAsMicroSeconds()/1000000,15);
			Serial.print(F("Time RANGE sent: ")); distantNode->timeRangeSent.print(); Serial.print(F(" / ")); Serial.println(distantNode->timeRangeSent.getAsMicroSeconds()/1000000,15);
			*/
		}
		Serial.print(F(" Range to device ")); Serial.print(distantNode->getVeryShortAddress());Serial.print(F(" is: ")); Serial.print(distance);
		Serial.print(F(" m, update frequency is: "));Serial.print(distantNode->getRangeFrequency()); Serial.println(F(" Hz"));

	}
	else if(msgtype == RANGE_REPORT){
		distantNode->setStatus(0);
		float curRange;
		memcpy(&curRange, _data+*ptr, 4);
		*ptr += 4;
		if(DEBUG){
			Serial.print(F("RANGE REPORT received from distantNode "));Serial.print(distantNode->getShortAddress(),HEX);
			//Serial.print(F("Individual bytes: "));Serial.print(*(_data+*ptr));Serial.print(F(" "));Serial.print(*(_data+*ptr+1));Serial.print(F(" "));Serial.print(*(_data+*ptr+2));Serial.print(F(" "));Serial.println(*(_data+*ptr+3));
		}
		Serial.print(F(" Range to device ")); Serial.print(distantNode->getVeryShortAddress());Serial.print(F(" is: ")); Serial.print(curRange);
		Serial.print(F(" m, update frequency is: "));Serial.print(distantNode->getRangeFrequency()); Serial.println(F(" Hz"));

	}
	// PROPERLY HANDLE RECEIVE FAILED MESSAGE (NOW IT JUST REPEATS THE PROTOCOL FROM SCRATCH)
	else if(msgtype == RECEIVE_FAILED){
		distantNode->setStatus(0);
		if(DEBUG){
			Serial.println("protocol failed at receive");
		}
	}
}

void ConnectedRangingClass::computeRangeAsymmetric(DW1000Device* myDistantDevice, DW1000Time* myTOF) {
	// asymmetric two-way ranging (more computation intense, less error prone)
	DW1000Time round1 = (myDistantDevice->timePollAckReceived-myDistantDevice->timePollSent).wrap();
	DW1000Time reply1 = (myDistantDevice->timePollAckSent-myDistantDevice->timePollReceived).wrap();
	DW1000Time round2 = (myDistantDevice->timeRangeReceived-myDistantDevice->timePollAckSent).wrap();
	DW1000Time reply2 = (myDistantDevice->timeRangeSent-myDistantDevice->timePollAckReceived).wrap();

	myTOF->setTimestamp((round1*round2-reply1*reply2)/(round1+round2+reply1+reply2));

	/*
	Serial.print("timePollAckReceived ");myDistantDevice->timePollAckReceived.print();
	Serial.print("timePollSent ");myDistantDevice->timePollSent.print();
	Serial.print("round1 "); round1.print();

	Serial.print("timePollAckSent ");myDistantDevice->timePollAckSent.print();
	Serial.print("timePollReceived ");myDistantDevice->timePollReceived.print();
	Serial.print("reply1 "); reply1.print();

	Serial.print("timeRangeReceived ");myDistantDevice->timeRangeReceived.print();
	Serial.print("timePollAckSent ");myDistantDevice->timePollAckSent.print();
	Serial.print("round2 "); round2.print();

	Serial.print("timeRangeSent ");myDistantDevice->timeRangeSent.print();
	Serial.print("timePollAckReceived ");myDistantDevice->timePollAckReceived.print();
	Serial.print("reply2 "); reply2.print();
	*/

}

void ConnectedRangingClass::updateSentTimes(){
	DW1000Node* distantNode;
	for (int i=0;i<_numNodes-1;i++){
		distantNode = &_networkNodes[i];
		if (distantNode->getStatus()==1){
			DW1000.getTransmitTimestamp(distantNode->timePollSent);
			if(DEBUG){
				Serial.print(F("POLL message sent to: ")); Serial.print(distantNode->getShortAddress(),HEX); Serial.print(F(" with timestamp: "));
				distantNode->timePollSent.print(); Serial.print(F(" / ")); Serial.println(distantNode->timePollSent.getAsMicroSeconds()/1000000,15);
			}
		}
		else if (distantNode->getStatus()==3){
			DW1000.getTransmitTimestamp(distantNode->timePollAckSent);
			if(DEBUG){
				Serial.print(F("POLL ACK message sent to: ")); Serial.print(distantNode->getShortAddress(),HEX); Serial.print(F(" with timestamp: "));
				distantNode->timePollAckSent.print(); Serial.print(F(" / ")); Serial.println(distantNode->timePollAckSent.getAsMicroSeconds()/1000000,15);

			}
		}
		else if (distantNode->getStatus()==5){
			DW1000Time rangeTime;
			DW1000.getTransmitTimestamp(rangeTime);
			if(DEBUG){
				Serial.print(F("RANGE message sent to: ")); Serial.print(distantNode->getShortAddress(),HEX); Serial.print(F(" with timestamp: "));
				rangeTime.print(); Serial.print(F(" / ")); Serial.println(rangeTime.getAsMicroSeconds()/1000000,15);
				//Serial.print(F("Stored RANGE time is: ")); distantNode->timeRangeSent.print(); Serial.print(F(" / ")); Serial.println(distantNode->timeRangeSent.getAsMicroSeconds()/1000000,15);
			}
		}
		else if (distantNode->getStatus()==7){
			DW1000Time rangeTime;
			DW1000.getTransmitTimestamp(rangeTime);
			if(DEBUG){
				Serial.print(F("RANGE REPORT message sent to: ")); Serial.print(distantNode->getShortAddress(),HEX); Serial.print(F(" with timestamp: "));
				rangeTime.print(); Serial.print(F(" / ")); Serial.println(rangeTime.getAsMicroSeconds()/1000000,15);
				//Serial.print(F("Stored RANGE time is: ")); distantNode->timeRangeSent.print(); Serial.print(F(" / ")); Serial.println(distantNode->timeRangeSent.getAsMicroSeconds()/1000000,15);
			}
		}
		if (distantNode->getStatus()==0){
			DW1000Time failTime;
			DW1000.getTransmitTimestamp(failTime);
			if(DEBUG){
				Serial.print(F("RECEIVE FAILED message sent to: ")); Serial.print(distantNode->getShortAddress(),HEX); Serial.print(F(" with timestamp: "));
				failTime.print(); Serial.print(F(" / ")); Serial.println(failTime.getAsMicroSeconds()/1000000,15);
			}
		}
	}
}

void ConnectedRangingClass::produceMessage(){
	uint16_t datapointer = 0;
	memcpy(_data,&_veryShortAddress,1);
	datapointer++;
	for(int i=0;i<_numNodes-1;i++){
		addMessageToData(&datapointer,&_networkNodes[i]);
	}


}

void ConnectedRangingClass::addMessageToData(uint16_t *ptr, DW1000Node *distantNode){
	switch(distantNode->getStatus()){
	case 0 : addPollMessage(ptr, distantNode); break;
	case 1 : addReceiveFailedMessage(ptr, distantNode); break;
	case 2 : addPollAckMessage(ptr, distantNode); break;
	case 3 : addReceiveFailedMessage(ptr, distantNode); break;
	case 4 : addRangeMessage(ptr, distantNode); break;
	case 5 : addReceiveFailedMessage(ptr, distantNode); break;
	case 6 : addRangeReportMessage(ptr, distantNode); break;
	case 7: addReceiveFailedMessage(ptr,distantNode); break;
	}

}

void ConnectedRangingClass::addPollMessage(uint16_t *ptr, DW1000Node *distantNode){
	distantNode->setStatus(1);
	byte toSend[2] = {distantNode->getVeryShortAddress(),POLL};
	memcpy(_data+*ptr,&toSend,2);
	*ptr+=2;
}

void ConnectedRangingClass::addPollAckMessage(uint16_t *ptr, DW1000Node *distantNode){
	distantNode->setStatus(3);
	byte toSend[2] = {distantNode->getVeryShortAddress(),POLL_ACK};
	memcpy(_data+*ptr,&toSend,2);
	*ptr+=2;
}

void ConnectedRangingClass::addRangeMessage(uint16_t *ptr, DW1000Node *distantNode){
	distantNode->setStatus(5);
	if(!_rangeSent){
		_rangeSent = true;
		// delay the next message sent because it contains a range message
		DW1000Time deltaTime = DW1000Time(DEFAULT_REPLY_DELAY_TIME,DW1000Time::MICROSECONDS);
		_rangeTime = DW1000.setDelay(deltaTime);
	}
	distantNode->timeRangeSent = _rangeTime;
	byte toSend[2] = {distantNode->getVeryShortAddress(),RANGE};
	memcpy(_data+*ptr,&toSend,2);
	*ptr += 2;
	distantNode->timePollSent.getTimestamp(_data+*ptr);
	*ptr += 5;
	distantNode->timePollAckReceived.getTimestamp(_data+*ptr);
	*ptr += 5;
	distantNode->timeRangeSent.getTimestamp(_data+*ptr);
	*ptr += 5;
}

void ConnectedRangingClass::addRangeReportMessage(uint16_t *ptr, DW1000Node *distantNode){
	distantNode->setStatus(7);
	byte toSend[2] = {distantNode->getVeryShortAddress(),RANGE_REPORT};
	memcpy(_data+*ptr,&toSend,2);
	*ptr += 2;
	float range = distantNode->getRange();
	if(DEBUG){
		//Serial.print(F("Range in range report message: ")); Serial.println(range);
	}
	memcpy(_data+*ptr,&range,4);
	*ptr += 4;
}

void ConnectedRangingClass::addReceiveFailedMessage(uint16_t *ptr, DW1000Node *distantNode){
	distantNode->setStatus(0);
	byte toSend[2] = {distantNode->getVeryShortAddress(),RECEIVE_FAILED};
	memcpy(_data+*ptr,&toSend,2);
	*ptr += 2;
}


// Reset functions
void ConnectedRangingClass::checkForReset(){
	if (millis() - _lastActivity > INACTIVITY_RESET_TIME){
		resetInactive();
	}
}

void ConnectedRangingClass::resetInactive(){
	if(DEBUG){
		Serial.println(F("Attempted to reset inactive device"));
	}
	receiver();
	noteActivity();
}

void ConnectedRangingClass::noteActivity(){
	_lastActivity = millis();
}

