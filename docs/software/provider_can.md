# provider_can

The `provider_can` package is the module that handles communicating with SONIA's subarine CAN devices.
SONIA's CAN communication standard is explained in another documentation. 

**available devices are:**

1. Barometer,
2. Led indicator,
3. Diver interface,
4. Hydrophones,
5. thrusters (8),
6. Power supply,
7. Mission switch,
8. Droppers,
9. Grabber,
10. Torpedo launcher.

## Overview

### Usage Example

A launch file exists for launching the `provider_can` package.
One you have compiled and sourced the `devel/setup.bash` file, you can simply
execute:

	roslaunch provider_can provider_can.launch

This will start the CAN communication system and open all the published topics.

## Global Architecture

### Class Diagrams

<img src="/assets/img/provider_can_class_diagram.png" alt="Class Diagram" style="width: 500px;"/>

## Configuration file

In the `/provider_can/config` directory, you can find `kvaser_conf.yaml` file which can be 
passed as a parameter to the ROS node. This file allows the user to modify some of
the configurations of the CAN drivers. The content of the yaml file is shown below.

* provider_can/driver/baudrate: 125
* provider_can/driver/device_id: 1
* provider_can/driver/unique_id: 1
* provider_can/driver/channel: 0
* provider_can/driver/usb_device: "KVaser"

**baudrate**: CAN baudrate values, in kHz. possible values are: 50, 62, 100, 125, 250 and 1000. 
Actual config is 125.  
**device_id** and **unique_id**: Computer ID. This is the CAN ID that will be used by the provider_can when sending broadcast messages.  
See Electrical section in the Wiki page for more inforamtions about IDs.  
**channel**: KVaser bus channel to use.  
**usb_device**: Actually, only one CAN device is possible, which is KVaser. All KVaser devices use the same librairies.  

## ROS Node

### Subscribed Topics

None. `provider_can` only publish topics.

### Published Topics

For each device initialized, there are 3 topics published:  
 
* DEVICE_msgs,
* DEVICE_notices,
* DEVICE_properties,

Where DEVICE is the name of the specific device (example: barometer_msgs)  

**DEVICE_msgs**

Theses topics will contain all device's specific informations that are
important during subarine's operation, such as speed for thrusters or
pressure for barometer.

**DEVICE_notices**

These topics will contain ping responses and faults for each devices.

**DEVICE_properties**

These topics will contain devices properties. These properties will  
be published at `provider_can` startup for each discovered and known devices.

**Properties are:**  

* uint16 firmware_version : `Signature of LPC chip`
* uint32 uc_signature : `may be RESET, SLEEP, WAKEUP or ISP`
* uint8 capabilities : `any data`
* uint8 device_data : `RTR send rate, in ms`
* uint16 poll_rate

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

As an example, if we want to send speed 50 to the port thruster, we would send this command in a command prompt:

`rosservice call /provider_can/send_can_message 2 1 0 50 ""`

* param device_id = 2 			// DEVICE_ID_actuators (constant defined in SendCanMessage.srv)
* param unique_id = 1 			// UNIQUE_ID_ACT_port_motor (constant defined in SendCanMessage.srv)
* param method_number = 0 		// METHOD_MOTOR_set_speed (constant defined in SendCanMessage.srv)
* param parameter_value = 50 	// speed value
* param string_param = "" 		// unused

See next sections for more explanations.


**Param `device_id`**

`device_id` represents the ID number of a specific class of devices, such as
actuator or sensors. The IDs are all defined as constants in `send_can_message` service.


**Param `unique_id`:**

unique_id represents the ID number of a specific device in a device_id class.
The IDs are all defined as constants in `send_can_message` service.

	
**Param `method_number` and `parameter_value`:**

`method_number` is the method you want to call from the specified device. Some methods  
are common to all devices. `parameter_value` is the only possible parameter of that method.
Note: diver_interface has a string parameter. All methods numbers are defined as contants in the service .srv file.



**Param `string_param`**

This parameter is only used by diver interface. It is the parameter
for the methods METHOD_DIVER_set_mission_string and METHOD_DIVER_set_state_string.
It changes the strings displayed on the interface.

**return**

can_send_message always return 0 or 1, depending if the device specified is 
present or not on the can bus.
