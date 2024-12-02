#include "componentlibrary.hpp"

using namespace rack;

struct PushButton5 : SvgSwitch {
    PushButton5() {
        // Load SVG files for each state
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/PushButton5_1.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/PushButton5_2.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/PushButton5_3.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/PushButton5_4.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/PushButton5_5.svg")));

        // Disable shadow for cleaner appearance
        shadow->opacity = 0.0f;
    }

    // Handle state cycling
     void onButton(const event::Button& e) override {
        if (e.action == GLFW_PRESS) {  // Only handle button press events
            // Cycle through the 5 states
            float currentValue = getParamQuantity()->getValue();
            int nextState = ((int)currentValue + 1) % 5; // Increment state and wrap back to 0
            getParamQuantity()->setValue((float)nextState); // Update state
        }
    }
};