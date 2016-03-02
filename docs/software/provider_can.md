# provider_can

The `provider_can` package is the module that handles communicating with SONIA's subarine CAN devices.
SONIA's CAN communication standard is explained in another documentation. 

Available devices are: 
Barometer,
Led indicator,
Diver interface,
Hydrophones,
thrusters (6),
Power supply,
Mission switch,
Droppers,
Grabber,
Torpedo launcher.

## Overview

### Usage Example

A launch file exists for launching the `provider_can` package.
One you have compiled and sourced the `devel/setup.bash` file, you can simply
execute:

	roslaunch provider_can provider_can.launch

This will start the CAN communication system and open all the published topics.

## Global Architecture

### Class Diagrams


### INS Flowcharts

## ROS Node

### Subscribed Topics

None. `provider_can` only publish topics.

### Published Topics

For each device initialized, there are 3 topics published:
DEVICE_msgs,
DEVICE_notices,
DEVICE_properties,
Where DEVICE is the name of the specific device (example: barometer_msgs)

####DEVICE_msgs

Theses topics will contain all device's specific informations that are
important during subarine's operation, such as speed for thrusters or
pressure for barometer.

####DEVICE_notices

These topics will contain ping responses and faults for each devices.

####DEVICE_properties

These topics will contain devices properties. These properties will
be published at `provider_can` startup for each discovered and known devices.

Properties are:

uint16 firmware_version
`Signature of LPC chip`
uint32 uc_signature
`may be RESET, SLEEP, WAKEUP or ISP`
uint8 capabilities
`any data`
uint8 device_data
`RTR send rate, in ms`
uint16 poll_rate

In Winter 2016 version of provider_can, poll_rate properties means nothing.
It is not actually implemented in ELE part. It will be later on.

### Services

`provider_can` offers 1 service, which is defined in `SendCanMessage.srv`
in `sonia_msgs` file.
The name of the service is `send_can_message`.

The service is common to all devices. You may set thrusters speed,
light intensity, diver_interface displayed text, etc. with this same
service. See the parameters for more informations.

### Parameters

This part is very important. It explains how to use the main service
of the provider_can, which is `send_can_message`.

`/provider_can/can/send_can_message` (`uint8 device_id`, `uint8 unique_id`,`uint8 method_number`,`float64 parameter_value`,`string string_param`) : returns uint8

#### Param `device_id`

`device_id` represents the ID number of a specific class of devices, such as
actuator or sensors. The IDs are all defined as constants in `send_can_message` service.

`device_id` list:

DEVICE_ID_controllers
DEVICE_ID_actuators
DEVICE_ID_markers
DEVICE_ID_sonars
DEVICE_ID_sensors
DEVICE_ID_power
DEVICE_ID_interfaces
DEVICE_ID_lights
DEVICE_ID_can2rs232

#### Param `unique_id`:

unique_id represents the ID number of a specific device in a device_id class.
The IDs are all defined as constants in `send_can_message` service.

`unique_id` list, separated by `device_id`:

DEVICE_ID_actuators
	UNIQUE_ID_ACT_port_motor
	UNIQUE_ID_ACT_starboard_motor
	UNIQUE_ID_ACT_front_heading_motor
	UNIQUE_ID_ACT_back_heading_motor
	UNIQUE_ID_ACT_front_depth_motor
	UNIQUE_ID_ACT_back_depth_motor
	UNIQUE_ID_ACT_grabber
DEVICE_ID_markers
	UNIQUE_ID_MARK_dropper
	UNIQUE_ID_MARK_launcher
DEVICE_ID_sonars
	UNIQUE_ID_SONAR_passive 
	UNIQUE_ID_SONAR_active
DEVICE_ID_sensors
	UNIQUE_ID_SENSORS_barometer
DEVICE_ID_power
	UNIQUE_ID_POWER_power_distribution
DEVICE_ID_interfaces
	UNIQUE_ID_INTERFACE_diver
	UNIQUE_ID_INTERFACE_mission_switch
	UNIQUE_ID_INTERFACE_cartenav_exceptions // will disappear
DEVICE_ID_lights
	UNIQUE_ID_LIGHT_bottom_light
	UNIQUE_ID_LIGHT_led_indicator
	
#### Param `method_number` and `parameter_value`:

`method_number` is the method you want to call from the specified device. Some methods
are common to all devices. `parameter_value` is the only possible parameter of that method.
Note: diver_interface has a string parameter.


`method_number` list and `parameter_value`:

uint8 METHOD_COMMON_ping_req				// No parameter
uint8 METHOD_COMMON_presence_check			// No parameter
uint8 METHOD_COMMON_get_properties			// No parameter

uint8 METHOD_BOTLIGTH_set_level				// Param: Level (0 to 100)

uint8 METHOD_PSU_pc_reset					// No param
uint8 METHOD_PSU_remote_kill				// Remote = 1, not remote = 0
uint8 METHOD_PSU_set_channel				// Param: Power channel (see methods parameters)
uint8 METHOD_PSU_clr_channel				// Param: Power channel (see methods parameters)

uint8 METHOD_MOTOR_set_speed				// Param: Speed (-100 to 100)

uint8 METHOD_GRABBER_port_set_target		// Param: target
uint8 METHOD_GRABBER_starboard_set_target 	// Param: target

uint8 METHOD_DIVER_set_mission_string		// Param: none. 
uint8 METHOD_DIVER_set_state_string			// Param: none.

uint8 METHOD_LED_set_mode					// Param: Mode (see methods parameters)
uint8 METHOD_LED_set_color					// Param: Color (see methods parameters)

`parameter_value` constants:

PSU_CHAN_BUS_12V1
PSU_CHAN_BUS_12V2
PSU_CHAN_PC
PSU_CHAN_MOTOR1
PSU_CHAN_MOTOR2
PSU_CHAN_MOTOR3
PSU_CHAN_DVL
PSU_CHAN_ACTUATORS
PSU_CHAN_LIGHT

LED_MODE_INDICATOR_OFF
LED_MODE_BLINK_MODE
LED_MODE_INDICATOR_ON
LED_MODE_RAINBOW

LED_COLOR_BLACK
LED_COLOR_RED
LED_COLOR_YELLOW
LED_COLOR_CYAN
LED_COLOR_GREEN 
LED_COLOR_WHITE
LED_COLOR_BLUE

#### Param `string_param` 

This parameter is only used by diver interface. It is the parameter
for the methods METHOD_DIVER_set_mission_string and METHOD_DIVER_set_state_string.
It changes the strings displayed on the interface.

#### return

can_send_message always return 0 or 1, depending if the device specified is 
present or not on the can bus.
