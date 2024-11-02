#include "plugin.hpp"


struct Rectifier : Module {
	enum ParamId {
		MANUAL_PARAM_PARAM,
		CV_PARAM_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		SIGNAL_INPUT_INPUT,
		CV_INPUT_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		FWR_OUTPUT_OUTPUT,
		FWRI_OUTPUT_OUTPUT,
		PHR_OUTPUT_OUTPUT,
		PHRI_OUTPUT_OUTPUT,
		NHR_OUTPUT_OUTPUT,
		NHRI_OUTPUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	Rectifier() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(MANUAL_PARAM_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CV_PARAM_PARAM, 0.f, 1.f, 0.f, "");
		configInput(SIGNAL_INPUT_INPUT, "");
		configInput(CV_INPUT_INPUT, "");
		configOutput(FWR_OUTPUT_OUTPUT, "");
		configOutput(FWRI_OUTPUT_OUTPUT, "");
		configOutput(PHR_OUTPUT_OUTPUT, "");
		configOutput(PHRI_OUTPUT_OUTPUT, "");
		configOutput(NHR_OUTPUT_OUTPUT, "");
		configOutput(NHRI_OUTPUT_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {
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

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.118, 21.0)), module, Rectifier::MANUAL_PARAM_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.118, 41.0)), module, Rectifier::CV_PARAM_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.362, 21.0)), module, Rectifier::SIGNAL_INPUT_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.362, 41.0)), module, Rectifier::CV_INPUT_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.362, 61.0)), module, Rectifier::FWR_OUTPUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.118, 61.0)), module, Rectifier::FWRI_OUTPUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.362, 81.0)), module, Rectifier::PHR_OUTPUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.118, 81.0)), module, Rectifier::PHRI_OUTPUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.362, 101.0)), module, Rectifier::NHR_OUTPUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.118, 101.0)), module, Rectifier::NHRI_OUTPUT_OUTPUT));
	}
};


Model* modelRectifier = createModel<Rectifier, RectifierWidget>("Rectifier");