#include "plugin.hpp"


struct Rectifier : Module {
	enum ParamId {
		MANUAL_PARAM,
		CV_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		SIGNAL_INPUT,
		CV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		FWR_OUTPUT,
		FWRI_OUTPUT,
		PHR_OUTPUT,
		PHRI_OUTPUT,
		NHR_OUTPUT,
		NHRI_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	Rectifier() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(MANUAL_PARAM, -10.f, 10.f, 0.f, "");
		configParam(CV_PARAM, 0.f, 1.f, 0.f, "");
		configInput(SIGNAL_INPUT, "");
		configInput(CV_INPUT, "");
		configOutput(FWR_OUTPUT, "");
		configOutput(FWRI_OUTPUT, "");
		configOutput(PHR_OUTPUT, "");
		configOutput(PHRI_OUTPUT, "");
		configOutput(NHR_OUTPUT, "");
		configOutput(NHRI_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {

		float manual_axis = params[MANUAL_PARAM].getValue();
		float cv_axis = params[CV_PARAM].getValue();

		float input = inputs[SIGNAL_INPUT].getVoltage();

		float full = input < manual_axis ? manual_axis + std::fabs(input) : input;
		float half_pos = input < manual_axis ? manual_axis + std::fabs(input) : input;
		float half_neg = input > manual_axis ? manual_axis - std::fabs(input) : input;

		// set output voltages
		outputs[PHR_OUTPUT].setVoltage(half_pos);
		outputs[FWR_OUTPUT].setVoltage(full);
		outputs[NHR_OUTPUT].setVoltage(half_neg);
		outputs[FWRI_OUTPUT].setVoltage(-full);
		outputs[PHRI_OUTPUT].setVoltage(-half_pos);
		outputs[NHRI_OUTPUT].setVoltage(-half_neg);

	}
};


struct RectifierWidget : ModuleWidget {
	RectifierWidget(Rectifier* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Rectifier.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.118, 21.0)), module, Rectifier::MANUAL_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.118, 41.0)), module, Rectifier::CV_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.362, 21.0)), module, Rectifier::SIGNAL_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.362, 41.0)), module, Rectifier::CV_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.362, 61.0)), module, Rectifier::FWR_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.118, 61.0)), module, Rectifier::FWRI_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.362, 81.0)), module, Rectifier::PHR_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.118, 81.0)), module, Rectifier::PHRI_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.362, 101.0)), module, Rectifier::NHR_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.118, 101.0)), module, Rectifier::NHRI_OUTPUT));
	}
};


Model* modelRectifier = createModel<Rectifier, RectifierWidget>("Rectifier");