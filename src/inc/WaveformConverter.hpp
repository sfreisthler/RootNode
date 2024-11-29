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
		float toSaw(float value, float freq, float sampleRate) {
			gate.set(value);


			if (gate.trailingEdge()) {
				//DEBUG("DEBUG: TRAILING EDGE");
				phase = 0.f;
			}

			// accumulate phase
			//DEBUG("DEBUG: Freq %f, Sample: %f", freq, sampleRate);
			phase += freq * sampleRate;

			if (phase >= 1.0f)
            	phase -= 1.0f;

			return 10.f * phase - 5.f;
		}
	

};