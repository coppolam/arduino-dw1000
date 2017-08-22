/*
 * DW1000Node.h
 *
 *  Created on: Aug 18, 2017
 *      Author: steven <stevenhelm@live.nl>
 */

#ifndef _DW1000Node_H_INCLUDED
#define _DW1000Node_H_INCLUDED

#include "DW1000Device.h"

class DW1000Node: public DW1000Device {
public:
	// Constructors and Destructor. All simply call the corresponding DW1000Device constructors.
	DW1000Node();
	DW1000Node(byte address[], byte shortAddress[]);
	DW1000Node(byte address[], boolean shortOne = true);
	~DW1000Node();

	// Getters
	uint8_t getStatus();

	// Setters
	void setStatus(uint8_t status);

	// Comparisons
	boolean operator==(const uint8_t cmp) const;
	boolean operator!=(const uint8_t cmp) const;



protected:
	uint8_t _status = 0;
	byte _veryShortAddress = 0;





















};











#endif
