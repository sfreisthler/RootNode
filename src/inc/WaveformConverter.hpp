//----------------------------------------------------------------------------
//	RootNode Plugin for VCV Rack - Waveform Converter
//	Converts a square wave to a sawtooth wave maintaining phase
//----------------------------------------------------------------------------
#pragma once
#include "GateProcessor.hpp"

class WaveformConverter {
	private:
		float phase = 0.f;
		GateProcessor gate;

	public:
		// converts input waveform to sawtooth wave
		float toSaw(float value, float freq, float sampleTime) {
			gate.set(value);


			if (gate.trailingEdge()) {
				phase = 0.f;
			}

			// accumulate phase
			phase += freq * sampleTime;

			if (phase >= 1.0f)
            	phase -= 1.0f;

			return 10.f * phase - 5.f;
		}
	

};