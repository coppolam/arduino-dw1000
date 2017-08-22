/*
 * ConnectedRanging.h
 *
 *  Created on: Aug 18, 2017
 *      Author: steven <stevenhelm@live.nl>
 */

//#ifndef _CONNECTEDRANGING_H_INCLUDED
//#define _CONNECTEDRANGING_H_INCLUDED




#include "DW1000.h"
#include "DW1000Time.h"
#include "DW1000Device.h"
#include "DW1000Mac.h"
#include "DW1000Node.h"


#define transmitDelay 1000
#define MAX_NODES 5

// messages used in the ranging protocol
#define POLL 0
#define POLL_ACK 1
#define RANGE 2
#define RANGE_REPORT 3
#define RANGE_FAILED 255

// number of bytes a certain message type takes up
#define POLL_SIZE 1
#define POLL_ACK_SIZE 1
#define RANGE_SIZE 13 // 1 + 12
#define RANGE_REPORT_SIZE 5 // 1 + 4

// reply time in us
#define DEFAULT_REPLY_DELAY_TIME 7000


class ConnectedRangingClass {

public:
	// data buffer
	static byte _data[MAX_LEN_DATA];

	// initialisation
	static void init(char longAddress[], uint8_t numNodes);
	static void init(uint8_t veryShortAddress, uint8_t numNodes);
	static void initDecawave(byte longAddress[], uint8_t numNodes, const byte mode[] = DW1000.MODE_LONGDATA_FAST_ACCURACY, uint16_t networkID=0xDECA,uint8_t myRST=9, uint8_t mySS=SS, uint8_t myIRQ=2);

	// set DW1000 in permanent receiving mode
	static void receiver();

	// transmit a byte array (maximum length of byte array is MAX_LEN_DATA)
	static void transmitData(byte datas[]);
	static void transmitData(char datas[]);
	static void transmitData(char datas[],uint16_t n);

	static void loopReceive();
	static void loopTransmit(char msg[],uint16_t n);
	static void loopTransmit();







	// handlers
	static void handleSent();
	static void handleReceived();

	// message parsing and handling
	static void handleMessage();
	static void incrementDataPointer(uint16_t *ptr);
	static void processMessage(uint8_t msgfrom,uint16_t *ptr);







protected:


	// pins on the arduino used to communicate with DW1000
	static uint8_t _RST;
	static uint8_t _SS;
	static uint8_t _IRQ;

	// addresses of current DW1000
	static byte _longAddress[LEN_EUI];
	static byte _veryShortAddress;

	// message sent/received state
	static volatile boolean _sentAck;
	static volatile boolean _receivedAck;

	// keeping track of send times
	static uint32_t _lastSent;

	// nodes to range with
	static DW1000Node _networkNodes[MAX_NODES];
	static uint8_t _numNodes;

	// initializing those nodes
	static void initNodes();

	// when it is time to send
	static boolean _timeToSend;











};

extern ConnectedRangingClass ConnectedRanging;

//#endif /* _CONNECTEDRANGING_H_INCLUDED */






