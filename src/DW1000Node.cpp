/*
 * DW1000Node.cpp
 *
 *  Created on: Aug 18, 2017
 *      Author: steven
 */



#include "DW1000Node.h"



DW1000Node::DW1000Node() : DW1000Device(){
}

DW1000Node::DW1000Node(byte address[], byte shortAddress[])  : DW1000Device(address, shortAddress){
}

DW1000Node::DW1000Node(byte address[], boolean shortOne = false) : DW1000Device(address,shortOne){
}

DW1000Node::~DW1000Node(){
}

uint8_t DW1000Node::getStatus(){
	return _status;
}

void DW1000Node::setStatus(uint8_t status){
	_status = status;
	return;
}
