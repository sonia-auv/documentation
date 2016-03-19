# provider_sonar

The `provider_sonar` package is the module handling the Tritech DST Micron sonar.  
The sonar works with a proprietary SeaNet RS-232 Protocol.

## Overview

### Usage Example

A launch file exists for launching the `provider_sonar` package.
One you have compiled and sourced the `devel/setup.bash` file, you can simply
execute:

	roslaunch provider_sonar provider_sonar.launch

This will start the sonar and open all the published topics.

## Global Architecture

### Class Diagrams

<img src="/assets/img/provider_sonar_class_diagram.png" alt="Class Diagram" style="width: 500px;"/>

### INS Flowcharts 

<img src="/assets/img/provider_sonar_state_machine.png" alt="State Machine" style="width: 400px;"/>
