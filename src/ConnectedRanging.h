/*
 * ConnectedRanging.h
 *
 *  Created on: Aug 18, 2017
 *      Author: steven
 */

//#ifndef _CONNECTEDRANGING_H_INCLUDED
//#define _CONNECTEDRANGING_H_INCLUDED


#include "DW1000.h"
#include "DW1000Time.h"
#include "DW1000Device.h"
#include "DW1000Mac.h"
#include "DW1000Node.h"




class ConnectedRangingClass {

public:
	void init(char longAddress[], uint16_t networkID=0xDECA,const byte mode[] = DW1000.MODE_LONGDATA_FAST_ACCURACY,uint8_t myRST=9, uint8_t mySS=SS, uint8_t myIRQ=2);










protected:



	static uint8_t _RST;
	static uint8_t _SS;
	static uint8_t _IRQ;
	static byte _longAddress[LEN_EUI];
	static byte _veryShortAddress;











};

extern ConnectedRangingClass ConnectedRanging;

//#endif /* _CONNECTEDRANGING_H_INCLUDED */
