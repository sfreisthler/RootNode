//------------------------------------------------------------------------
//  /^M^\ Count Modula Plugin for VCV Rack - Frequency Divider
//  Copyright (C) 2019  Adam Verspaget
//----------------------------------------------------------------------------
#pragma once
#include "GateProcessor.hpp"

#define COUNT_UP 1
#define COUNT_DN 2

struct FrequencyDivider {
	int count = 0;
	int N = 0;
	int maxN = 20;
	int countMode = COUNT_DN;
	
	bool phase = false;
	GateProcessor gate;

	// process the given clock value and return the current divider state
	bool process(float clk) {

		// process the clock;
		gate.set(clk);
		
		if (gate.anyEdge()) {
				count++;

				// for count up mode, flip the phase and reset the count at the end of the count
				if ((countMode == COUNT_UP && count == N))
					phase = !phase;
				
				if (count >= N) {
					// we've hit the counter
					count = 0;
				}
		
			// for count down mode, flip the phase and reset the count at the start
			if ((countMode == COUNT_DN && count == 0))
				phase = !phase;
		}
		
		return phase;
	}
	
	void setN (int in) {
		N = clamp(in, 1, maxN);
	}
	
	// set the counter mode to up or down. This controls whether the phase changes on the first clock or the last clock of the count
	void setCountMode(int mode) {
		switch(mode) {
			case COUNT_DN:
			case COUNT_UP:
				countMode = mode;
				break;
		}
	}
	
	// set the maximum division value - limited to 1-64 as I don't see the point in larger divisions at the moment
	void setMaxN(int max) {
		maxN = max;
		if (maxN < 1)
			maxN = 1;
		else if (maxN > 64)
			maxN = 64;
	}
	
	// reset the counter
	void reset () {
		countMode = COUNT_DN;
		count = -1;
		N = 0;
		phase = false;
		gate.reset();
	}
};