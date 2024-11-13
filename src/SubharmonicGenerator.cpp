#include "plugin.hpp"

#include <cmath>

struct VoltageControlledOscillator {
    float phase = 0.f;
    float freq = 0.f;

    dsp::MinBlepGenerator<16, 16, float> sawMinBlep;
    dsp::MinBlepGenerator<16, 16, float> sqrMinBlep;

    float sawValue = 0.f;
    float sqrValue = 0.f;

    void process(float deltaTime) {
        // Advance phase
        float deltaPhase = simd::clamp(freq * deltaTime, 0.f, 0.35f);
        phase += deltaPhase;

       // Wrap phase to stay within [0, 1]
        if (phase >= 1.0f)
            phase -= 1.0f;

        // Calculate square wave (±1) based on phase
        sqrValue = phase < 0.5f ? 1.f : -1.f;
        sqrValue += sqrMinBlep.process();

        // Calculate sawtooth wave (linear ramp from -1 to 1)
        sawValue = 2.f * phase - 1.f;
        sawValue += sawMinBlep.process();
    }

    float saw() {
        return sawValue;
    }

    float sqr() {
        return sqrValue;
    }
};


struct SubharmonicGenerator : Module {
	enum ParamId {
		FREQ_PARAM,
		WAVEFORM_PARAM,
		SUB_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		INPUTS_LEN
	};
	enum OutputId {
		OSC_OUTPUT,
		SUB_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	VoltageControlledOscillator oscillator1;
	VoltageControlledOscillator sub1;

	SubharmonicGenerator() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(FREQ_PARAM, 262.f, 4186.f, 0.f, ""); // frequency range from middle c to four octaves above
		configSwitch(WAVEFORM_PARAM, 0.f, 2.f, 0.f, "Waveform", {"Saw", "Square<-Saw", "Square"});
		configParam(SUB_PARAM, 1.f, 16.f, 1.f, "");
		configOutput(OSC_OUTPUT, "");
		configOutput(SUB_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {
		float osc1Freq = params[FREQ_PARAM].getValue();
		float waveform = params[WAVEFORM_PARAM].getValue();
		float sub1Selector = std::floor(params[SUB_PARAM].getValue());
		float sub1Freq = osc1Freq / sub1Selector;

		// add cv input here

		osc1Freq = clamp(osc1Freq, 0.f, args.sampleRate / 2.f);
		sub1Freq = clamp(sub1Freq, 0.f, args.sampleRate / 2.f);

		oscillator1.freq = osc1Freq;
		oscillator1.process(args.sampleTime);

		sub1.freq = sub1Freq;
		sub1.process(args.sampleTime);

		if (waveform == 2.0f and outputs[OSC_OUTPUT].isConnected()) {
			outputs[OSC_OUTPUT].setVoltage(5.f * oscillator1.sqr());
			outputs[SUB_OUTPUT].setVoltage(5.f * sub1.sqr());
		} else if (waveform == 1.0f and outputs[OSC_OUTPUT].isConnected()) {
			outputs[OSC_OUTPUT].setVoltage(5.f * oscillator1.sqr());
			outputs[SUB_OUTPUT].setVoltage(5.f * sub1.saw());
		} else if (waveform == 0.0f and outputs[OSC_OUTPUT].isConnected()) {
			outputs[OSC_OUTPUT].setVoltage(5.f * oscillator1.saw());
			outputs[SUB_OUTPUT].setVoltage(5.f * sub1.saw());
		}

	}
};

struct SubharmonicGeneratorWidget : ModuleWidget {
	SubharmonicGeneratorWidget(SubharmonicGenerator* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/SubharmonicGenerator.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 26.497)), module, SubharmonicGenerator::FREQ_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 71.551)), module, SubharmonicGenerator::SUB_PARAM));

		addParam(createParamCentered<CKSSThree>(mm2px(Vec(15.24, 60.0)), module, SubharmonicGenerator::WAVEFORM_PARAM));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 49.024)), module, SubharmonicGenerator::OSC_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 94.078)), module, SubharmonicGenerator::SUB_OUTPUT));
	}
};


Model* modelSubharmonicGenerator = createModel<SubharmonicGenerator, SubharmonicGeneratorWidget>("SubharmonicGenerator");