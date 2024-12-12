#include <cmath>
#include <vector>
#include <algorithm>

const double A4_FREQUENCY = 440.0;
const int A4_MIDI_NOTE = 69; 
const double C4_FREQUENCY = 261.626;

// converts freq in hertz to midi note for 12 tone system
double frequencyToMidi12(double frequency) {
    return A4_MIDI_NOTE + 12.0 * std::log2(frequency / A4_FREQUENCY);
}

// converts midi note to frequency value for 12 tone system
double midiToFrequency12(double midiNote) {
    return A4_FREQUENCY * std::pow(2.0, (midiNote - A4_MIDI_NOTE) / 12.0);
}

// converts freq in hertz to midi note for 8 tone system
double frequencyToMidi8(double frequency) {
    return A4_MIDI_NOTE + 8.0 * std::log2(frequency / A4_FREQUENCY);
}

// converts midi note to frequency value for 8 tone system
double midiToFrequency8(double midiNote) {
    return A4_FREQUENCY * std::pow(2.0, (midiNote - A4_MIDI_NOTE) / 8.0);
}

double quantize12ET(double frequency) {
    double midiNote = frequencyToMidi12(frequency);
    double roundedMidi = std::round(midiNote);
    return midiToFrequency12(roundedMidi);
}

double quantize8ET(double frequency) {
	double midiNote = frequencyToMidi8(frequency);
	double roundedMidi = std::round(midiNote);
	return midiToFrequency8(roundedMidi);

}

double quantize12JI(double frequency) {
	float ratios[13] = { 1.f / 1.f, 16.f / 15.f, 9.f / 8.f, 6.f / 5.f, 5.f / 4.f, 4.f / 3.f,
									45.f / 32.f, 3.f / 2.f, 8.f / 5.f, 5.f / 3.f, 9.f / 5.f, 15.f / 8.f,
									2.f / 1.f};
	double relativeFrequency = frequency / C4_FREQUENCY;
	double octaveFactor = 1.f;

    while (relativeFrequency >= 2.0) {
        relativeFrequency /= 2.0;
		octaveFactor *= 2.f;
    }

    // Find the closest ratio
    double minError = std::numeric_limits<double>::max();
    double closestRatio = 0.0;
    std::string closestDegree;
    for (size_t i = 0; i < sizeof(ratios); ++i) {
        double error = std::abs(relativeFrequency - ratios[i]);
        if (error < minError) {
            minError = error;
            closestRatio = ratios[i];
        }
    }

    // Calculate the frequency in just intonation
    return C4_FREQUENCY * closestRatio * octaveFactor;
}

double quantize8JI(double frequency) {
	float ratios[8] = { 1.f / 1.f, 9.f / 8.f, 5.f / 4.f, 4.f / 3.f,
							3.f / 2.f, 5.f / 3.f, 15.f / 8.f,
							2.f / 1.f};
	double relativeFrequency = frequency / C4_FREQUENCY;
	double octaveFactor = 1.f;

    while (relativeFrequency >= 2.0) {
        relativeFrequency /= 2.0;
		octaveFactor *= 2.f;
    }

    // Find the closest ratio
    double minError = std::numeric_limits<double>::max();
    double closestRatio = 0.0;
    std::string closestDegree;
    for (size_t i = 0; i < sizeof(ratios); ++i) {
        double error = std::abs(relativeFrequency - ratios[i]);
        if (error < minError) {
            minError = error;
            closestRatio = ratios[i];
        }
    }

    // Calculate the frequency in just intonation
    return C4_FREQUENCY * closestRatio * octaveFactor;
}