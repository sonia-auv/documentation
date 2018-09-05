#!/bin/bash
gpu_name=$(cat /sys/bus/i2c/drivers/ina3221x/0-0040/iio_device/rail_name_0)
gpu_current=$(cat /sys/bus/i2c/devices/0-0040/iio_device/in_current0_input)
gpu_voltage=$(cat /sys/bus/i2c/devices/0-0040/iio_device/in_voltage0_input)
gpu_power=$(cat /sys/bus/i2c/devices/0-0040/iio_device/in_power0_input)cpu_name=$(cat /sys/bus/i2c/drivers/ina3221x/0-0041/iio_device/rail_name_1)
cpu_current=$(cat /sys/bus/i2c/devices/0-0041/iio_device/in_current1_input)
cpu_voltage=$(cat /sys/bus/i2c/devices/0-0041/iio_device/in_voltage1_input)
cpu_power=$(cat /sys/bus/i2c/devices/0-0041/iio_device/in_power1_input)soc_name=$(cat /sys/bus/i2c/drivers/ina3221x/0-0040/iio_device/rail_name_1)
soc_current=$(cat /sys/bus/i2c/devices/0-0040/iio_device/in_current1_input)
soc_voltage=$(cat /sys/bus/i2c/devices/0-0040/iio_device/in_voltage1_input)
soc_power=$(cat /sys/bus/i2c/devices/0-0040/iio_device/in_power1_input)ddr_name=$(cat /sys/bus/i2c/drivers/ina3221x/0-0041/iio_device/rail_name_2)
ddr_current=$(cat /sys/bus/i2c/devices/0-0041/iio_device/in_current2_input)
ddr_voltage=$(cat /sys/bus/i2c/devices/0-0041/iio_device/in_voltage2_input)
ddr_power=$(cat /sys/bus/i2c/devices/0-0041/iio_device/in_power2_input)in_name=$(cat /sys/bus/i2c/drivers/ina3221x/0-0041/iio_device/rail_name_0)
in_current=$(cat /sys/bus/i2c/devices/0-0041/iio_device/in_current0_input)
in_voltage=$(cat /sys/bus/i2c/devices/0-0041/iio_device/in_voltage0_input)
in_power=$(cat /sys/bus/i2c/devices/0-0041/iio_device/in_power0_input)total_current=$((gpu_current+$soc_current+$in_current+$cpu_current+$ddr_current))
total_voltage=$((gpu_voltage+$soc_voltage+$in_voltage+$cpu_voltage+$ddr_voltage))
total_power=$((gpu_power+$soc_power+$in_power+$cpu_power+$ddr_power))

echo " ********* GPU ***********"
echo ""
echo "GPU:${gpu_name}"
echo "Current:${gpu_current} mA"
echo "Voltage:${gpu_voltage} mV"
echo "Power  :${gpu_power}   mW"
echo ""
echo " ********* CPU ***********"
echo ""
echo "CPU:${cpu_name}"
echo "Current:${cpu_current} mA"
echo "Voltage:${cpu_voltage} mV"
echo "Power  :${cpu_power}   mW"
echo ""
echo " ********* SOC ***********"
echo ""
echo "SOC:${soc_name}"
echo "Current:${soc_current} mA"
echo "Voltage:${soc_voltage} mV"
echo "Power  :${soc_power}   mW"
echo ""
echo " ********* DDR ***********"
echo ""
echo "DDR:${ddr_name}"
echo "Current:${ddr_current} mA"
echo "Voltage:${ddr_voltage} mV"
echo "Power  :${ddr_power}   mW"
echo ""
echo " ********* IN  ***********"
echo ""
echo "IN:${in_name}"
echo "Current:${in_current} mA"
echo "Voltage:${in_voltage} mV"
echo "Power  :${in_power}   mW"
echo ""
echo " ********* TOTAL ***********"
echo ""
echo "Current:${total_current} mA"
echo "Voltage:${total_voltage} mV"
echo "Power  :${total_power}  mW"
echo ""
echo "***************************"
