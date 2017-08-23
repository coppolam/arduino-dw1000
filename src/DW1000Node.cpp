/*
 * DW1000Node.cpp
 *
 *  Created on: Aug 18, 2017
 *      Author: steven <stevenhelm@live.nl>
 */



#include "DW1000Node.h"



DW1000Node::DW1000Node() : DW1000Device(){
	_veryShortAddress = _shortAddress[0];
}

DW1000Node::DW1000Node(byte address[], byte shortAddress[])  : DW1000Device(address, shortAddress){
	_veryShortAddress = address[0];
}

DW1000Node::DW1000Node(byte address[], boolean shortOne) : DW1000Device(address,shortOne){
	_veryShortAddress = address[0];
}

DW1000Node::~DW1000Node(){
}

uint8_t DW1000Node::getStatus(){
	return _status;
}

byte DW1000Node::getVeryShortAddress(){
	return _veryShortAddress;
}

void DW1000Node::setStatus(uint8_t status){
	_status = status;
	return;
}

boolean DW1000Node::operator==(const uint8_t cmp) const {
	return _veryShortAddress==cmp;
}

boolean DW1000Node::operator!=(const uint8_t cmp) const {
	return _veryShortAddress!=cmp;
}

void DW1000Node::printNode(){
	Serial.print(F("Node with Very Short Address: "));
	Serial.print(_veryShortAddress,HEX);
	Serial.print(F(" and Long Address: "));
	for(int j=0; j<8;j++){
		Serial.print(_ownAddress[j],HEX);
	}
	Serial.print(F(" and Status: "));
	Serial.println(_status);

}

