#include "plugin.hpp"
#include <cmath>
#include "inc/WaveformConverter.hpp"
#include "inc/FrequencyDivider.hpp"
#include "inc/Utility.hpp"
#include "inc/GateProcessor.hpp"

struct SquareWaveGenerator {
    float phase = 0.f;
    float freq = 0.f;
	float dutyCycle = 0.5f;

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

        // Calculate square wave (±5) based on phase
        sqrValue = phase < dutyCycle ? 5.f : -5.f;
        sqrValue += sqrMinBlep.process();
    }

    float sqr() {
        return sqrValue;
    }

};


struct SubharmonicGenerator : Module {
	enum ParamId {
		ENUMS(OSC_PARAM, 2),
		ENUMS(SUB_PARAM, 4),
		ENUMS(OSC_LEVEL_PARAM, 2),
		ENUMS(SUB_LEVEL_PARAM, 4),
		ENUMS(WAVEFORM_PARAM, 2),
		PARAMS_LEN
	};
	enum InputId {
		VCO1_INPUT,
		VCO1_SUB_INPUT,
		VCO1_PWM_INPUT,
		VCO2_INPUT,
		VCO2_SUB_INPUT,
		VCO2_PWM_INPUT,
		VCA_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		VCO1_OUTPUT,
		VCO1_SUB1_OUTPUT,
		VCO1_SUB2_OUTPUT,
		VCA_OUTPUT,
		VCO2_OUTPUT,
		VCO2_SUB1_OUTPUT,
		VCO2_SUB2_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	SquareWaveGenerator oscillators[2];
	WaveformConverter converters[6];
	FrequencyDivider dividers[4];

	SubharmonicGenerator() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		
		// configure patchbay I/O
		configInput(VCO1_INPUT, "");
		configInput(VCO1_SUB_INPUT, "");
		configInput(VCO1_PWM_INPUT, "");
		configInput(VCO2_INPUT, "");
		configInput(VCO2_SUB_INPUT, "");
		configInput(VCO2_PWM_INPUT, "");
		configInput(VCA_INPUT, "");
		configOutput(VCO1_OUTPUT, "");
		configOutput(VCO1_SUB1_OUTPUT, "");
		configOutput(VCO1_SUB2_OUTPUT, "");
		configOutput(VCA_OUTPUT, "");
		configOutput(VCO2_OUTPUT, "");
		configOutput(VCO2_SUB1_OUTPUT, "");
		configOutput(VCO2_SUB2_OUTPUT, "");

		// configure oscillator and mixer parameters
		for (int i = 0; i < 4; i++) {
			configParam(SUB_PARAM + i, 1.f, 16.f, 1.f, "");
			configParam(SUB_LEVEL_PARAM + i, 0.f, 1.f, 0.f, "");

			if (i < 2) {
				configParam(OSC_LEVEL_PARAM + i, 0.f, 1.f, 0.f, "");
				configParam(OSC_PARAM + i, 262.f, 4186.f, 0.f, "");
				configSwitch(WAVEFORM_PARAM + i, 0.f, 2.f, 0.f, "Waveform", {"Saw", "Square<-Saw", "Square"});
			}
		}
	}

	void process(const ProcessArgs& args) override {
		float vco1_cv = pow(2.f, inputs[VCO1_INPUT].getVoltage() - 5.f);
		float vco2_cv = pow(2.f, inputs[VCO2_INPUT].getVoltage() - 5.f);
		int vco1_sub_cv = std::floor(rescale(inputs[VCO1_SUB_INPUT].getVoltage() - 5.f, -5, 5, 0.f, 16.f));
		int vco2_sub_cv = std::floor(rescale(inputs[VCO2_SUB_INPUT].getVoltage() - 5.f, -5, 5, 0.f, 16.f));
		float vco1_pwm = rescale(inputs[VCO1_PWM_INPUT].getVoltage() - 5.f, -5, 5, 0.01, 0.99);
		float vco2_pwm = rescale(inputs[VCO2_PWM_INPUT].getVoltage() - 5.f, -5, 5, 0.01, 0.99);

		oscillators[0].dutyCycle = 0.5f;
		oscillators[1].dutyCycle = 0.5f;

		if (inputs[VCO1_PWM_INPUT].isConnected()) 
			oscillators[0].dutyCycle = vco1_pwm;

		if (inputs[VCO2_PWM_INPUT].isConnected()) 
			oscillators[1].dutyCycle = vco2_pwm;


		for (int i = 0; i < 4; i++) {
			dividers[i].setN(std::floor(params[SUB_PARAM + i].getValue()));
			dividers[i].setMaxN(16);

			if (i < 2) {
				if (inputs[VCO1_INPUT].isConnected()) {
					oscillators[i].freq = params[OSC_PARAM + i].getValue() * vco1_cv;
				} else {
					oscillators[i].freq = params[OSC_PARAM + i].getValue();
				}

				if (inputs[VCO2_INPUT].isConnected() and i == 1)
					oscillators[i].freq = params[OSC_PARAM + i].getValue() * vco2_cv;
		
				oscillators[i].process(args.sampleTime);
			}
		}

		if (inputs[VCO1_SUB_INPUT].isConnected()) {
			dividers[0].setN(vco1_sub_cv);
			dividers[1].setN(vco1_sub_cv);
		}

		if (inputs[VCO2_SUB_INPUT].isConnected()) {
			dividers[2].setN(vco2_sub_cv);
			dividers[3].setN(vco2_sub_cv);
		}

		// Set outputs based on osc1 switch
		float osc1 = oscillators[0].sqr();
		float sub11 = boolToAudio(dividers[0].process(osc1));
		float sub12 = boolToAudio(dividers[1].process(osc1));
		float out = 0.f;
		switch ((int) params[WAVEFORM_PARAM].getValue()) {
			case 0:
				outputs[VCO1_OUTPUT].setVoltage(converters[0].toSaw(osc1, oscillators[0].freq, args.sampleTime) * params[OSC_LEVEL_PARAM].getValue());
				outputs[VCO1_SUB1_OUTPUT].setVoltage(converters[1].toSaw(sub11, oscillators[0].freq / dividers[0].N, args.sampleTime) * params[SUB_LEVEL_PARAM].getValue());
				outputs[VCO1_SUB2_OUTPUT].setVoltage(converters[2].toSaw(sub12, oscillators[0].freq / dividers[1].N, args.sampleTime) * params[SUB_LEVEL_PARAM + 1].getValue());
				break;
			case 1:
				outputs[VCO1_OUTPUT].setVoltage(osc1 * params[OSC_LEVEL_PARAM].getValue());
				outputs[VCO1_SUB1_OUTPUT].setVoltage(converters[1].toSaw(sub11, oscillators[0].freq / dividers[0].N, args.sampleTime) * params[SUB_LEVEL_PARAM].getValue());
				outputs[VCO1_SUB2_OUTPUT].setVoltage(converters[2].toSaw(sub12, oscillators[0].freq / dividers[1].N, args.sampleTime) * params[SUB_LEVEL_PARAM + 1].getValue());
				break;
			case 2:
				outputs[VCO1_OUTPUT].setVoltage(osc1 * params[OSC_LEVEL_PARAM].getValue());
				outputs[VCO1_SUB1_OUTPUT].setVoltage(sub11 * params[SUB_LEVEL_PARAM].getValue());
				outputs[VCO1_SUB2_OUTPUT].setVoltage(sub12 * params[SUB_LEVEL_PARAM + 1].getValue());
				break;
		}

		// Set outputs based on osc2 switch
		float osc2 = oscillators[1].sqr();
		float sub21 = boolToAudio(dividers[2].process(osc2));
		float sub22 = boolToAudio(dividers[3].process(osc2));
		switch ((int) params[WAVEFORM_PARAM + 1].getValue()) {
			case 0:
				outputs[VCO2_OUTPUT].setVoltage(converters[3].toSaw(oscillators[1].sqr(), oscillators[1].freq, args.sampleTime) * params[OSC_LEVEL_PARAM + 1].getValue());
				outputs[VCO2_SUB1_OUTPUT].setVoltage(converters[4].toSaw(sub21, oscillators[1].freq / dividers[2].N, args.sampleTime) * params[SUB_LEVEL_PARAM + 2].getValue());
				outputs[VCO2_SUB2_OUTPUT].setVoltage(converters[5].toSaw(sub22, oscillators[1].freq / dividers[3].N, args.sampleTime) * params[SUB_LEVEL_PARAM + 3].getValue());
				break;
			case 1:
				outputs[VCO2_OUTPUT].setVoltage(oscillators[1].sqr() * params[OSC_LEVEL_PARAM + 1].getValue());
				outputs[VCO2_SUB1_OUTPUT].setVoltage(converters[4].toSaw(sub21, oscillators[1].freq / dividers[2].N, args.sampleTime) * params[SUB_LEVEL_PARAM + 2].getValue());
				outputs[VCO2_SUB2_OUTPUT].setVoltage(converters[5].toSaw(sub22, oscillators[1].freq / dividers[3].N, args.sampleTime) * params[SUB_LEVEL_PARAM + 3].getValue());
				break;
			case 2:
				outputs[VCO2_OUTPUT].setVoltage(osc2 * params[OSC_LEVEL_PARAM + 1].getValue());
				outputs[VCO2_SUB1_OUTPUT].setVoltage(sub21 * params[SUB_LEVEL_PARAM + 2].getValue());
				outputs[VCO2_SUB2_OUTPUT].setVoltage(sub22 * params[SUB_LEVEL_PARAM + 3].getValue());
				break;

		}

		out += outputs[VCO1_OUTPUT].getVoltage();
		out += outputs[VCO2_OUTPUT].getVoltage();
		out += outputs[VCO1_SUB1_OUTPUT].getVoltage();
		out += outputs[VCO1_SUB2_OUTPUT].getVoltage();
		out += outputs[VCO2_SUB1_OUTPUT].getVoltage();
		out += outputs[VCO2_SUB2_OUTPUT].getVoltage();

		outputs[VCA_OUTPUT].setVoltage(clamp(out, -11.2f, 11.2f));
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

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(24.955, 28.285)), module, SubharmonicGenerator::OSC_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(79.254, 28.285)), module, SubharmonicGenerator::OSC_PARAM + 1));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(11.026, 54.593)), module, SubharmonicGenerator::SUB_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.883, 54.593)), module, SubharmonicGenerator::SUB_PARAM + 1));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(65.325, 54.593)), module, SubharmonicGenerator::SUB_PARAM + 2));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(93.183, 54.593)), module, SubharmonicGenerator::SUB_PARAM + 3));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(24.955, 80.901)), module, SubharmonicGenerator::OSC_LEVEL_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(79.254, 80.901)), module, SubharmonicGenerator::OSC_LEVEL_PARAM + 1));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(11.026, 107.209)), module, SubharmonicGenerator::SUB_LEVEL_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.883, 107.209)), module, SubharmonicGenerator::SUB_LEVEL_PARAM + 1));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(65.325, 107.209)), module, SubharmonicGenerator::SUB_LEVEL_PARAM + 2));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(93.183, 107.209)), module, SubharmonicGenerator::SUB_LEVEL_PARAM + 3));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(131.478, 28.285)), module, SubharmonicGenerator::VCO1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(149.434, 28.285)), module, SubharmonicGenerator::VCO1_SUB_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(167.391, 28.285)), module, SubharmonicGenerator::VCO1_PWM_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(113.522, 80.901)), module, SubharmonicGenerator::VCO2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(131.478, 80.901)), module, SubharmonicGenerator::VCO2_SUB_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(149.434, 80.901)), module, SubharmonicGenerator::VCO2_PWM_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(167.391, 80.901)), module, SubharmonicGenerator::VCA_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(113.522, 54.593)), module, SubharmonicGenerator::VCO1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(131.478, 54.593)), module, SubharmonicGenerator::VCO1_SUB1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(149.434, 54.593)), module, SubharmonicGenerator::VCO1_SUB2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(167.391, 54.593)), module, SubharmonicGenerator::VCA_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(113.522, 107.209)), module, SubharmonicGenerator::VCO2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(131.478, 107.209)), module, SubharmonicGenerator::VCO2_SUB1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(149.434, 107.209)), module, SubharmonicGenerator::VCO2_SUB2_OUTPUT));

		addParam(createParamCentered<CKSSThree>(mm2px(Vec(11.026, 28.285)), module, SubharmonicGenerator::WAVEFORM_PARAM));
		addParam(createParamCentered<CKSSThree>(mm2px(Vec(93.183, 28.285)), module, SubharmonicGenerator::WAVEFORM_PARAM + 1));
	}
};


Model* modelSubharmonicGenerator = createModel<SubharmonicGenerator, SubharmonicGeneratorWidget>("SubharmonicGenerator");