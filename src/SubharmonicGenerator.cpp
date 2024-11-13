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
		FREQ1_PARAM,
		FREQ2_PARAM,
		SUB11_PARAM,
		SUB21_PARAM,
		SUB12_PARAM,
		SUB22_PARAM,
		WAVEFORM_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		INPUTS_LEN
	};
	enum OutputId {
		OSC1_OUTPUT,
		OSC2_OUTPUT,
		SUB11_OUTPUT,
		SUB21_OUTPUT,
		SUB12_OUTPUT,
		SUB22_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	VoltageControlledOscillator oscillator1;
	VoltageControlledOscillator sub11;

	SubharmonicGenerator() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(FREQ1_PARAM, 262.f, 4186.f, 0.f, "");
		configParam(FREQ2_PARAM, 262.f, 4186.f, 0.f, "");
		configParam(SUB11_PARAM, 1.f, 16.f, 1.f, "");
		configParam(SUB21_PARAM, 1.f, 16.f, 1.f, "");
		configParam(SUB12_PARAM, 1.f, 16.f, 1.f, "");
		configParam(SUB22_PARAM, 1.f, 16.f, 1.f, "");
		configOutput(OSC1_OUTPUT, "");
		configOutput(OSC2_OUTPUT, "");
		configOutput(SUB11_OUTPUT, "");
		configOutput(SUB21_OUTPUT, "");
		configOutput(SUB12_OUTPUT, "");
		configOutput(SUB22_OUTPUT, "");

		configSwitch(WAVEFORM_PARAM, 0.f, 2.f, 0.f, "Waveform", {"Saw", "Square<-Saw", "Square"});
	}

	void process(const ProcessArgs& args) override {
		float osc1Freq = params[FREQ1_PARAM].getValue();
		float waveform = params[WAVEFORM_PARAM].getValue();
		float sub11Selector = std::floor(params[SUB11_PARAM].getValue());
		float sub11Freq = osc1Freq / sub11Selector;

		// add cv input here

		osc1Freq = clamp(osc1Freq, 0.f, args.sampleRate / 2.f);
		sub11Freq = clamp(sub11Freq, 0.f, args.sampleRate / 2.f);

		oscillator1.freq = osc1Freq;
		oscillator1.process(args.sampleTime);

		sub11.freq = sub11Freq;
		sub11.process(args.sampleTime);

		if (waveform == 2.0f and outputs[OSC1_OUTPUT].isConnected()) {
			outputs[OSC1_OUTPUT].setVoltage(5.f * oscillator1.sqr());
			outputs[SUB11_OUTPUT].setVoltage(5.f * sub11.sqr());
		} else if (waveform == 1.0f and outputs[OSC1_OUTPUT].isConnected()) {
			outputs[OSC1_OUTPUT].setVoltage(5.f * oscillator1.sqr());
			outputs[SUB11_OUTPUT].setVoltage(5.f * sub11.saw());
		} else if (waveform == 0.0f and outputs[OSC1_OUTPUT].isConnected()) {
			outputs[OSC1_OUTPUT].setVoltage(5.f * oscillator1.saw());
			outputs[SUB11_OUTPUT].setVoltage(5.f * sub11.saw());
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

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(18.585, 13.216)), module, SubharmonicGenerator::FREQ1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(41.89, 13.216)), module, SubharmonicGenerator::FREQ2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(18.827, 54.043)), module, SubharmonicGenerator::SUB11_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(42.133, 54.043)), module, SubharmonicGenerator::SUB21_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(18.827, 94.87)), module, SubharmonicGenerator::SUB12_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(42.133, 94.87)), module, SubharmonicGenerator::SUB22_PARAM));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.827, 33.63)), module, SubharmonicGenerator::OSC1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.133, 33.63)), module, SubharmonicGenerator::OSC2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.827, 74.457)), module, SubharmonicGenerator::SUB11_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.133, 74.457)), module, SubharmonicGenerator::SUB21_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.827, 115.284)), module, SubharmonicGenerator::SUB12_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.133, 115.284)), module, SubharmonicGenerator::SUB22_OUTPUT));

		addParam(createParamCentered<CKSSThree>(mm2px(Vec(30, 60.0)), module, SubharmonicGenerator::WAVEFORM_PARAM));
	}
};


Model* modelSubharmonicGenerator = createModel<SubharmonicGenerator, SubharmonicGeneratorWidget>("SubharmonicGenerator");