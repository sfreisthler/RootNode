# RootNode

The majority of files in this project are located in two folders. `res` contains static svg resources for the module and `src` contains the code logic. More detail about the files in these folders will be described below.

## `src`

### `\inc`
Contains files that encapsulate core functionality that powers the subharmonic generator. `FrequencyDivider.hpp`, `GateProcessor.hpp`, and `Utility.hpp` are all sourced from Adam Verspaget and the Count Modula plugin.
\
\
`GateProcessor.hpp` \
A utility for processing and maintaining state for gate signals in VCV Rack using a Schmitt Trigger. The `GateProccesor` class has functionality for high, low, and edge detection.

`FrequencyDivider.hpp` \
Provides functionaliy to divide an input clock into sub-frequencies based on provided integer value. The `FrequencyDivider` class maintains phase between input clock and output waveform.

`Utility.hpp` \
Contains macros to convert input boolean values to corresponding output voltages based on use-case.

`Quantize.cpp` \
Encapsulates logic to convert a note frequency to it's corresponding value in different tuning systems.

`WaveformConverter.hpp` \
Converts input square wave into a triangle wave maintaining phase.

### `\components`
`RootNodeComponents.hpp` \
Defines custom RootNode components. `PushButton5` is a button with 5 distinct states that cycle through on each click.

`SubharmonicGenerator.cpp`
The main code for the SubharmonicGenerator module that ties together logic from all other src files. The `SubharmonicGenerator` class handles interpreting input values and setting output values for all module I/O and parameter points.

## `res`
### `\components`
Continas files for custom tree shaped push button. One file for each state color.
\
\
`SubharmonicGenerator.svg` \
Defines the panel design for the vcv module when displayed in VCV Rack.
