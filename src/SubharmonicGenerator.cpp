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
		WAVEFORM1_PARAM,
		WAVEFORM2_PARAM,
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
	VoltageControlledOscillator oscillator2;
	VoltageControlledOscillator sub11;
	VoltageControlledOscillator sub12;
	VoltageControlledOscillator sub21;
	VoltageControlledOscillator sub22;

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

		configSwitch(WAVEFORM1_PARAM, 0.f, 2.f, 0.f, "Waveform", {"Saw", "Square<-Saw", "Square"});
		configSwitch(WAVEFORM2_PARAM, 0.f, 2.f, 0.f, "Waveform", {"Saw", "Square<-Saw", "Square"});
	}

	void process(const ProcessArgs& args) override {
		float waveform1 = params[WAVEFORM1_PARAM].getValue();
		float waveform2 = params[WAVEFORM2_PARAM].getValue();

		float osc1Freq = params[FREQ1_PARAM].getValue();
		float sub11Selector = std::floor(params[SUB11_PARAM].getValue());
		float sub11Freq = osc1Freq / sub11Selector;
		float sub12Selector = std::floor(params[SUB12_PARAM].getValue());
		float sub12Freq = osc1Freq / sub12Selector;

		float osc2Freq = params[FREQ2_PARAM].getValue();
		float sub21Selector = std::floor(params[SUB21_PARAM].getValue());
		float sub21Freq = osc2Freq / sub21Selector;
		float sub22Selector = std::floor(params[SUB22_PARAM].getValue());
		float sub22Freq = osc2Freq / sub22Selector;

		// add cv input here

		osc1Freq = clamp(osc1Freq, 0.f, args.sampleRate / 2.f);
		osc2Freq = clamp(osc2Freq, 0.f, args.sampleRate / 2.f);

		sub11Freq = clamp(sub11Freq, 0.f, args.sampleRate / 2.f);
		sub12Freq = clamp(sub12Freq, 0.f, args.sampleRate / 2.f);
		sub21Freq = clamp(sub21Freq, 0.f, args.sampleRate / 2.f);
		sub22Freq = clamp(sub22Freq, 0.f, args.sampleRate / 2.f);

		oscillator1.freq = osc1Freq;
		oscillator1.process(args.sampleTime);

		oscillator2.freq = osc2Freq;
		oscillator2.process(args.sampleTime);


		sub11.freq = sub11Freq;
		sub11.process(args.sampleTime);
		sub12.freq = sub12Freq;
		sub12.process(args.sampleTime);
		sub21.freq = sub21Freq;
		sub21.process(args.sampleTime);
		sub22.freq = sub22Freq;
		sub22.process(args.sampleTime);

		if (waveform1 == 2.0f and outputs[OSC1_OUTPUT].isConnected()) {
			outputs[OSC1_OUTPUT].setVoltage(5.f * oscillator1.sqr());
			outputs[SUB11_OUTPUT].setVoltage(5.f * sub11.sqr());
			outputs[SUB12_OUTPUT].setVoltage(5.f * sub12.sqr());
		} else if (waveform1 == 1.0f and outputs[OSC1_OUTPUT].isConnected()) {
			outputs[OSC1_OUTPUT].setVoltage(5.f * oscillator1.sqr());
			outputs[SUB11_OUTPUT].setVoltage(5.f * sub11.saw());
			outputs[SUB12_OUTPUT].setVoltage(5.f * sub12.saw());
		} else if (waveform1 == 0.0f and outputs[OSC1_OUTPUT].isConnected()) {
			outputs[OSC1_OUTPUT].setVoltage(5.f * oscillator1.saw());
			outputs[SUB11_OUTPUT].setVoltage(5.f * sub11.saw());
			outputs[SUB12_OUTPUT].setVoltage(5.f * sub12.saw());
		}

		if (waveform2 == 2.0f and outputs[OSC2_OUTPUT].isConnected()) {
			outputs[OSC2_OUTPUT].setVoltage(5.f * oscillator2.sqr());
			outputs[SUB21_OUTPUT].setVoltage(5.f * sub21.sqr());
			outputs[SUB22_OUTPUT].setVoltage(5.f * sub22.sqr());
		} else if (waveform2 == 1.0f and outputs[OSC2_OUTPUT].isConnected()) {
			outputs[OSC2_OUTPUT].setVoltage(5.f * oscillator2.sqr());
			outputs[SUB21_OUTPUT].setVoltage(5.f * sub21.saw());
			outputs[SUB22_OUTPUT].setVoltage(5.f * sub22.saw());
		} else if (waveform2 == 0.0f and outputs[OSC2_OUTPUT].isConnected()) {
			outputs[OSC2_OUTPUT].setVoltage(5.f * oscillator2.saw());
			outputs[SUB21_OUTPUT].setVoltage(5.f * sub21.saw());
			outputs[SUB22_OUTPUT].setVoltage(5.f * sub22.saw());
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

		addParam(createParamCentered<CKSSThree>(mm2px(Vec(30, 60.0)), module, SubharmonicGenerator::WAVEFORM1_PARAM));
		addParam(createParamCentered<CKSSThree>(mm2px(Vec(30, 90.0)), module, SubharmonicGenerator::WAVEFORM2_PARAM));
	}
};


Model* modelSubharmonicGenerator = createModel<SubharmonicGenerator, SubharmonicGeneratorWidget>("SubharmonicGenerator");