#include "plugin.hpp"


struct VoltageControlledOscillator {
    float phase = 0.f;
	float sub1Phase = 0.f;
	float sub2Phase = 0.f;
    float freq = 0.f;
	float sub1Selector = 1.f;
	float sub2Selector = 1.f;

    dsp::MinBlepGenerator<16, 16, float> sawMinBlep;
    dsp::MinBlepGenerator<16, 16, float> sqrMinBlep;

    float sawValue = 0.f;
    float sqrValue = 0.f;
	float sub1SqrValue = 0.f;
	float sub1SawValue = 0.f;
	float sub2SqrValue = 0.f;
	float sub2SawValue = 0.f;
	float totalValue = 0.f;

    void process(float deltaTime) {
        // Advance phase
        float deltaPhase = simd::clamp(freq * deltaTime, 0.f, 0.35f);
		float deltaSub1Phase = simd::clamp(freq * deltaTime / sub1Selector, 0.f, 0.35f);
		float deltaSub2Phase = simd::clamp(freq * deltaTime / sub2Selector, 0.f, 0.35f);
        phase += deltaPhase;
		sub1Phase += deltaSub1Phase;
		sub2Phase += deltaSub2Phase;

       // Wrap phase to stay within [0, 1]
        if (phase >= 1.0f)
            phase -= 1.0f;
		
		if (sub1Phase >= 1.0f)
			sub1Phase -= 1.0f;

		if (sub2Phase >= 1.0f)
			sub2Phase -= 1.0f;

        // Calculate square wave (±1) based on phase
        sqrValue = phase < 0.5f ? 1.f : -1.f;
        sqrValue += sqrMinBlep.process();

        // Calculate sawtooth wave (linear ramp from -1 to 1)
        sawValue = 2.f * phase - 1.f;
        sawValue += sawMinBlep.process();

		sub1SqrValue = sub1Phase < 0.5f ? 1.f : -1.f;
		sub2SqrValue = sub2Phase < 0.5f ? 1.f : -1.f;

		sub1SawValue = 2.f * sub1Phase - 1.f;
		sub2SawValue = 2.f * sub2Phase - 1.f;
			
    }

    float saw() {
        return sawValue;
    }

    float sqr() {
        return sqrValue;
    }

	float sub1Saw() {
		return sub1SawValue;
	}

	float sub1Sqr() {
		return sub1SqrValue;
	}

	float sub2Saw() {
		return sub2SawValue;
	}

	float sub2Sqr() {
		return sub2SqrValue;
	}

	float total() {
		return totalValue;
	}
};

struct SubharmonicGenerator : Module {
	enum ParamId {
		FREQ1_PARAM,
		FREQ2_PARAM,
		SUB11_PARAM,
		SUB12_PARAM,
		SUB21_PARAM,
		SUB22_PARAM,
		OSC1_LEVEL_PARAM,
		OSC2_LEVEL_PARAM,
		SUB11_LEVEL_PARAM,
		SUB12_LEVEL_PARAM,
		SUB21_LEVEL_PARAM,
		SUB22_LEVEL_PARAM,
		WAVEFORM1_PARAM,
		WAVEFORM2_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		INPUTS_LEN
	};
	enum OutputId {
		MIXED_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	VoltageControlledOscillator oscillator1;
	VoltageControlledOscillator oscillator2;

	SubharmonicGenerator() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(FREQ1_PARAM, 262.f, 4186.f, 0.f, "");
		configParam(FREQ2_PARAM, 262.f, 4186.f, 0.f, "");
		configParam(SUB11_PARAM, 1.f, 16.f, 1.f, "");
		configParam(SUB21_PARAM, 1.f, 16.f, 1.f, "");
		configParam(SUB12_PARAM, 1.f, 16.f, 1.f, "");
		configParam(SUB22_PARAM, 1.f, 16.f, 1.f, "");
		configParam(OSC1_LEVEL_PARAM, 0.f, 1.f, 0.f, "");
		configParam(OSC2_LEVEL_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SUB11_LEVEL_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SUB12_LEVEL_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SUB21_LEVEL_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SUB22_LEVEL_PARAM, 0.f, 1.f, 0.f, "");
		configOutput(MIXED_OUTPUT, "");

		configSwitch(WAVEFORM1_PARAM, 0.f, 2.f, 0.f, "Waveform", {"Saw", "Square<-Saw", "Square"});
		configSwitch(WAVEFORM2_PARAM, 0.f, 2.f, 0.f, "Waveform", {"Saw", "Square<-Saw", "Square"});
	}

	void process(const ProcessArgs& args) override {
		float waveform1 = params[WAVEFORM1_PARAM].getValue();
		float waveform2 = params[WAVEFORM2_PARAM].getValue();

		float osc1Freq = params[FREQ1_PARAM].getValue();
		float sub11Selector = std::floor(params[SUB11_PARAM].getValue());
		float sub12Selector = std::floor(params[SUB12_PARAM].getValue());

		float osc2Freq = params[FREQ2_PARAM].getValue();
		float sub21Selector = std::floor(params[SUB21_PARAM].getValue());
		float sub22Selector = std::floor(params[SUB22_PARAM].getValue());

		float osc1Level = params[OSC1_LEVEL_PARAM].getValue();
		float osc2Level = params[OSC2_LEVEL_PARAM].getValue();
		float sub11Level = params[SUB11_LEVEL_PARAM].getValue();
		float sub12Level = params[SUB12_LEVEL_PARAM].getValue();
		float sub21Level = params[SUB21_LEVEL_PARAM].getValue();
		float sub22Level = params[SUB22_LEVEL_PARAM].getValue();

		// add cv input here

		osc1Freq = clamp(osc1Freq, 0.f, args.sampleRate / 2.f);
		osc2Freq = clamp(osc2Freq, 0.f, args.sampleRate / 2.f);

		oscillator1.freq = osc1Freq;
		oscillator1.process(args.sampleTime);
		oscillator1.sub1Selector = sub11Selector;
		oscillator1.sub2Selector = sub12Selector;

		oscillator2.freq = osc2Freq;
		oscillator2.process(args.sampleTime);
		oscillator2.sub1Selector = sub21Selector;
		oscillator2.sub2Selector = sub22Selector;

		if (waveform1 == 2.f) {
			oscillator1.totalValue = (oscillator1.sqr() * osc1Level + oscillator1.sub1Sqr() * sub11Level + oscillator1.sub2Sqr() * sub12Level) / 3.f;
		} else if (waveform1 == 1.f) {
			oscillator1.totalValue = (oscillator1.sqr() * osc1Level + oscillator1.sub1Saw() * sub11Level + oscillator1.sub2Saw() * sub12Level) / 3.f;
		} else if (waveform1 == 0.f) {
			oscillator1.totalValue = (oscillator1.saw() * osc1Level + oscillator1.sub1Saw() * sub11Level + oscillator1.sub2Saw() * sub12Level) / 3.f;
		}

		if (waveform2 == 2.f) {
			oscillator2.totalValue = (oscillator2.sqr() * osc2Level + oscillator2.sub1Sqr() * sub21Level + oscillator2.sub2Sqr() * sub22Level) / 3.f;
		} else if (waveform2 == 1.f) {
			oscillator2.totalValue = (oscillator2.sqr() * osc2Level + oscillator2.sub1Saw() * sub21Level + oscillator2.sub2Saw() * sub22Level) / 3.f;
		} else {
			oscillator2.totalValue = (oscillator2.saw() * osc2Level + oscillator2.sub1Saw() * sub21Level + oscillator2.sub2Saw() * sub22Level) / 3.f;
		}

		if (outputs[MIXED_OUTPUT].isConnected()) {
			outputs[MIXED_OUTPUT].setVoltage(5.f * (oscillator1.total() + oscillator2.total()) / 2.f);
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

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.919, 14.137)), module, SubharmonicGenerator::FREQ1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(90.831, 14.137)), module, SubharmonicGenerator::FREQ2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(16.82, 35.164)), module, SubharmonicGenerator::SUB11_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44.678, 35.164)), module, SubharmonicGenerator::SUB12_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.902, 35.164)), module, SubharmonicGenerator::SUB21_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(104.76, 35.164)), module, SubharmonicGenerator::SUB22_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(31.089, 67.535)), module, SubharmonicGenerator::OSC1_LEVEL_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.001, 67.535)), module, SubharmonicGenerator::OSC2_LEVEL_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(16.99, 88.562)), module, SubharmonicGenerator::SUB11_LEVEL_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44.848, 88.562)), module, SubharmonicGenerator::SUB12_LEVEL_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(77.072, 88.562)), module, SubharmonicGenerator::SUB21_LEVEL_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(104.93, 88.562)), module, SubharmonicGenerator::SUB22_LEVEL_PARAM));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(16.99, 115.284)), module, SubharmonicGenerator::MIXED_OUTPUT));

		addParam(createParamCentered<CKSSThree>(mm2px(Vec(13, 10)), module, SubharmonicGenerator::WAVEFORM1_PARAM));
		addParam(createParamCentered<CKSSThree>(mm2px(Vec(108.9, 10.0)), module, SubharmonicGenerator::WAVEFORM2_PARAM));
	}
};


Model* modelSubharmonicGenerator = createModel<SubharmonicGenerator, SubharmonicGeneratorWidget>("SubharmonicGenerator");