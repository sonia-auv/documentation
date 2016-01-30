# Package: Provider DVL

## Global Architecture

## Configurations

## Topics ROS

### `/provider_dvl/pd0_packet`

The `pd0_packet topics contains all the informations of the DVL.
It is being sent from the DVL at every signals.

The specifications of the PD0 packets can be found on the Teledyne Documentation (c.f. Github).
It is composed of the other messages of the DVL and does not contain any raw data. The specifications for all the other messages can also be found on the documentation.

	# sonia_msgs/PD0Packet
	std_msgs/Header header	  uint32 seq	  time stamp	  string frame_id	sonia_msgs/DeviceInfo device_info	  std_msgs/Header header	    uint32 seq	    time stamp	    string frame_id	  uint8 fw_version	  uint8 fw_revision	  uint64 cpu_board_serno	  uint16 system_configuration	  uint8 beam_count	  sonia_msgs/Sensors available_sensors	    std_msgs/Header header	      uint32 seq	      time stamp	      string frame_id	    bool calculates_speed_of_sound	    bool depth	    bool yaw	    bool pitch	    bool roll	    bool salinity	    bool temperature	sonia_msgs/AcquisitionConfiguration acquisition_conf	  ...	sonia_msgs/OutputConfiguration output_conf
	  ...	sonia_msgs/Status status	  ...	sonia_msgs/CellReadings cell_readings	  ...	sonia_msgs/BottomTrackingConfiguration bottom_tracking_conf	  ...	sonia_msgs/BottomTracking bottom_tracking	  ...

!!!Note
	As the message AcquisitionConfigure to BottomTracking are also provided on other topics described below, you will be able to see the description for each one through their specific section.

### `/provider_dvl/acquisition_conf`

???

	# sonia_msgs/AcquisitionConfiguration
	std_msgs/Header header	  uint32 seq	  time stamp	  string frame_id	sonia_msgs/Sensors used_sensors	  std_msgs/Header header	    uint32 seq	    time stamp	    string frame_id	  bool calculates_speed_of_sound	  bool depth	  bool yaw	  bool pitch	  bool roll	  bool salinity	  bool temperature	uint8 lag_duration	uint8 cell_count	uint8 profiling_mode	uint8 low_correlation_threshold	uint8 code_repetition_count	uint16 pings_per_ensemble	float32 cell_length	float32 blank_after_transmit_distance	float32 water_layer_min_ping_threshold	float32 water_layer_velocity_threshold	uint64 time_between_ping_groups	float32 yaw_alignment	float32 yaw_bias	float32 first_cell_distance	float32 transmit_pulse_length	uint8 water_layer_start	uint8 water_layer_end	uint8 false_target_threshold	bool low_latency_trigger	float32 transmit_lag_distance	bool narrow_bandwidth_mode	uint8 base_frequency_index

### `/provider_dvl/output_conf`

???

	# sonia_msgs/OutputConfiguration
	uint8 BEAM=0	uint8 INSTRUMENT=1	uint8 SHIP=2	uint8 EARTH=3	std_msgs/Header header	  uint32 seq	  time stamp	  string frame_id	uint8 coordinate_system	bool use_attitude	bool use_3beam_solution	bool use_bin_mapping

### `/provider_dvl/status`

???

	# sonia_msgs/Status
	std_msgs/Header header	  uint32 seq	  time stamp	  string frame_id	uint32 seq	uint64 time	geometry_msgs/Quaternion orientation	  float64 x	  float64 y	  float64 z	  float64 w	geometry_msgs/Vector3 stddev_orientation	  float64 x	  float64 y	  float64 z	float32 depth	float32 speed_of_sound	float32 salinity	float32 temperature	float32 pressure	float32 pressure_variance	uint8[8] adc_channels	uint64 min_preping_wait	uint16 self_test_result	uint32 status_word

### `/provider_dvl/cell_readings`

???

	# sonia_msgs/CellReadings
	std_msgs/Header header	  uint32 seq	  time stamp	  string frame_id	uint64 time	sonia_msgs/CellReading[] readings	  std_msgs/Header header	    uint32 seq	    time stamp	    string frame_id	  float32[4] velocity	  float32[4] correlation	  float32[4] intensity	  float32[4] quality

### `/provider_dvl/bottom_tracking_conf`

???

	# sonia_msgs/BottomTrackingConfiguration
	std_msgs/Header header	  uint32 seq	  time stamp	  string frame_id	uint16 ping_per_ensemble	uint16 delay_before_reacquiring	float32 correlation_threshold	float32 evaluation_threshold	float32 good_ping_threshold	uint8 mode	float32 max_velocity_error	float32 max_tracking_depth	uint8 gain

### `/provider_dvl/bottom_tracking`

???

	# sonia_msgs/BottomTracking
	std_msgs/Header header	  uint32 seq	  time stamp	  string frame_id	float64 time	float64[4] range	float64[4] velocity	float64[4] correlation	float64[4] evaluation	float64[4] good_ping_ratio	float64[4] rssi

### `/provider_dvl/twist`

???

	# geometry_msgs/TwistWithCovarianceStamped
	std_msgs/Header header	  uint32 seq	  time stamp	  string frame_id	geometry_msgs/TwistWithCovariance twist	  geometry_msgs/Twist twist	    geometry_msgs/Vector3 linear	      float64 x	      float64 y	      float64 z	    geometry_msgs/Vector3 angular	      float64 x	      float64 y	      float64 z	  float64[36] covariance

