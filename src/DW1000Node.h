/*
 * DW1000Node.h
 *
 *  Created on: Aug 18, 2017
 *      Author: steven <stevenhelm@live.nl>
 */

#ifndef _DW1000Node_H_INCLUDED
#define _DW1000Node_H_INCLUDED

#include "DW1000Device.h"

#define INIT_STATUS 0
#define POLL_SENT 1
#define POLL_RECEIVED 2
#define POLL_ACK_SENT 3
#define POLL_ACK_RECEIVED 4
#define RANGE_SENT 5
#define RANGE_RECEIVED 6
#define RANGE_REPORT_SENT 7

class DW1000Node: public DW1000Device {
public:
	// Constructors and Destructor. All simply call the corresponding DW1000Device constructors.
	DW1000Node();
	DW1000Node(byte address[], byte shortAddress[]);
	DW1000Node(byte address[], boolean shortOne = true);
	~DW1000Node();

	// Getters
	uint8_t getStatus();
	byte getVeryShortAddress();

	// Setters
	void setStatus(uint8_t status);

	// Comparisons
	boolean operator==(const uint8_t cmp) const;
	boolean operator!=(const uint8_t cmp) const;

	// Utility functions
	void printNode();
	float getRangeFrequency();

protected:
	uint8_t _status = 0;
	byte _veryShortAddress = 0;
	uint32_t _successRanges = 0;
	uint32_t _rangeTimer = millis();
	float _rangeFreq = 0;





















};











#endif
