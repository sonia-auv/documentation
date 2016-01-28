# <a name="title"></a> Doppler Velocity Logger

[Acoustic Doppler current profiler](https://en.wikipedia.org/wiki/Acoustic_Doppler_current_profiler#Bottom_tracking)

The DVL is a device that measure the speed of water. It is largely used in underwater navigation.
In the case of our AUV, the DVL is a device with a half-sphere-shaped placed at the bottom of the frame. 

The DVL is using the Doppler effect in order to calculate the speed of the water around itself.

From Wikipedia:
*"The Doppler effect (or Doppler shift) is the change in frequency of a wave (or other periodic event) for an observer moving relative to its source. It is named after the Austrian physicist Christian Doppler, who proposed it in 1842 in Prague."*

The DVL sends an accoustic wave at a specific frequency. This wave is going to be reflected by the bodies that it will meet. In our case, this is generally a wall or the floor of the pool.
The DVL will then receive the reflected wave and it will be able to observe a delta between the sent wave and the received one.

From this observation, we can have sort out the following statements:

- If F_r < f_o, the submarine is moving away from the body that reflected the wave.
- If F_r > F_o, the submarine is moving toward the body that reflected the wave.


