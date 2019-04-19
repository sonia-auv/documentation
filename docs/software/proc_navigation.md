# proc_navigation

The `proc_navigation` package is the module that provides the system with the localisation and the altitude of the submarine.

It is based on an [Extended Kalman Filter](https://en.wikipedia.org/wiki/Extended_Kalman_filter) and will take the data from the Inertial Measurement Unit (IMU)( the Magnetometer and the Barometer) and the Doppler Velocity Log (DVL).

## Overview

### Usage Example

A launch file exists for launching the `proc_navigation` package.
Once you have compiled and sourced the `devel/setup.bash` file, you can simply
execute:

	roslaunch proc_navigation proc_navigation.launch

This will start the navigation system and open all the published topics.

## Global Architecture

### Class Diagrams

<img src="/assets/img/ins_class_diagram.png" alt="Class Diagram" style="width: 500px;"/>

### INS Flowcharts

The Kalman contains two steps of process. The first one is called the Initialization and will allow the system to have its initial states matrix.

The second step is called the Update and is the actual process that will sort out the robot states with error reduction.

The Initial state is very simple and consists on acquiring data during a certain period, calculate the mean of it in order to set the initial states of the system:

<img src="/assets/img/ins_init_flowchart.png" alt="Init Flowchart" style="width: 300px;"/>

Here is the flowchart for the Kalman Update state:

<img src="/assets/img/ins_update_flowchart.png" alt="Update Flowchart" style="width: 500px;"/>

## ROS Node

### Subscribed Topics

`/provider_can/barometer/pressure` (`std_msgs::Float64`)

The absolute pressure in [PA].

`/provider_dvl/twist` (`geometry_msgs::TwistWithCovarianceStamped`)

The velocity of the robot.
We only consider the linear velocity in the twist message.

`/provider_imu/imu` (`sensor_msgs::Imu`)

A message from the IMU.
We consider the linear acceleration and the angular velocity for the position.

`/provider_imu/mag` (`sensor_msgs::MagneticField`)

The magnetic field (commonly provided by the IMU).
We use this information to calculate the Gravity vector.

### Published Topics

???

### Services

???

### Parameters

`/proc_navigation/ekf/t_init` (`float`, default value: `0.5`)

The time of the initialisation step

`/proc_navigation/ekf/active_gravity` (`bool`, default value: `true`)  
`/proc_navigation/ekf/active_mag` (`bool`, default value: `true`)  
`/proc_navigation/ekf/active_dvl` (`bool`, default value: `true`)  
`/proc_navigation/ekf/active_baro` (`bool`, default value: `true`)  

States if the devices are activated in the EKF

`/proc_navigation/ekf/sigma_meas_gravity` (`float`, default value: `10.0`)  
`/proc_navigation/ekf/sigma_meas_mag` (`float`, default value: `5.0`)  
`/proc_navigation/ekf/sigma_meas_dvl_x` (`float`, default value: `3.0`)  
`/proc_navigation/ekf/sigma_meas_dvl_y` (`float`, default value: `3.0`)  
`/proc_navigation/ekf/sigma_meas_dvl_z` (`float`, default value: `5.0`)  
`/proc_navigation/ekf/sigma_meas_baro` (`float`, default value: `10.0`)  

Measure variances

`/proc_navigation/ekf/sigma0_pos_x` (`float`, default value: `0.01`)  
`/proc_navigation/ekf/sigma0_pos_y` (`float`, default value: `0.01`)  
`/proc_navigation/ekf/sigma0_pos_z` (`float`, default value: `0.01`)  
`/proc_navigation/ekf/sigma0_vel_x` (`float`, default value: `0.01`)  
`/proc_navigation/ekf/sigma0_vel_y` (`float`, default value: `0.01`)  
`/proc_navigation/ekf/sigma0_vel_z` (`float`, default value: `0.01`)  
`/proc_navigation/ekf/sigma0_rho_x` (`float`, default value: `0.01`)  
`/proc_navigation/ekf/sigma0_rho_y` (`float`, default value: `0.01`)  
`/proc_navigation/ekf/sigma0_rho_z` (`float`, default value: `0.01`)  
`/proc_navigation/ekf/sigma0_bias_acc` (`float`, default value: `0.001`)  
`/proc_navigation/ekf/sigma0_bias_gyr` (`float`, default value: `0.001`)  
`/proc_navigation/ekf/sigma0_bias_baro` (`float`, default value: `0.001`)  

Kalman initialization states uncertainty

`/proc_navigation/ekf/sigma__meas_acc` (`float`, default value: `10`)  
`/proc_navigation/ekf/sigma_meas_gyr` (`float`, default value: `0.01`)  
`/proc_navigation/ekf/sigma_walk_bias_acc` (`float`, default value: `0.001`)  
`/proc_navigation/ekf/sigma_walk_bias_gyr` (`float`, default value: `0.001`)  
`/proc_navigation/ekf/sigma_walk_bias_baro` (`float`, default value: `0.001`)  

Propagation uncertainty

`/proc_navigation/ekf/l_pd` (`std::vector<double>`, default value: `[0.0, 0.0, 0.15]`)  

Distance from IMU to devices in body frame (NED)
IMU - DVL

`/proc_navigation/ekf/l_pp` (`std::vector<double>`, default value: `[0.0, -0.10, -0.5]`)  

Distance from IMU to devices in body frame (NED)
IMU - DVL

`/proc_navigation/ekf/crit_station_acc` (`float`, default value: `1.0`)  
`/proc_navigation/ekf/crit_station_norm` (`float`, default value: `1.0`)  

Stationnary State Detection

`/proc_navigation/device_sign/imu/x` (`int`, default value: `1`)  
`/proc_navigation/device_sign/imu/y` (`int`, default value: `-1`)  
`/proc_navigation/device_sign/imu/z` (`int`, default value: `-1`)  
`/proc_navigation/device_sign/mag/x` (`int`, default value: `1`)  
`/proc_navigation/device_sign/mag/y` (`int`, default value: `-1`)  
`/proc_navigation/device_sign/mag/z` (`int`, default value: `-1`)  

According to the device orientation, the sign can change, set the values here
