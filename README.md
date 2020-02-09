# argo3_closest_light
## INTRODUCTION ##
This project uses the argos-simulator to simulate robots who try to find their next charging stations.
## Getting started ##
To compile & build the code, run
```mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
cd ..
```

To run the example, type
```
argos3 -c experiments/closest.argos
```

## Configuration ##

You have many options to change the experiment by configuring the file ``closest.argos``.
Most configurations should be self-explanatory, however, configuring the usage of sensors is a bit more tricky:
See the following line:
```
        <gl lights="true" botsFc="1" botsOa="true" walls="true"/>
```
The first options, ``lights="xyz"`` will tell the simulation whether the robots will know the position of the lights
(and therefore the charging stations). If it is "true", the positions will be knows. If it is set to "false" the robots will try to
detect the lights by using their "lights-sensors".

The second option, ``botsFc="x"`` has three possible settings:
* 0 means that robots will try to detect other robots via their "proximity-sesnors" to calculate the next charging station
* 1 means that robots will **know** all other robots to calculate the next charging station
* 2 means that robots will ignore the other robots and go to the nearest charging station

The last two options, ``botsOa="xyz"`` and ``walls="xyz"`` work similarly to the first option: If set to true, robots will
know all robots/walls for **obstacle avoidance**. If set to false, they will use their "proximity-sensors" to detect them.

**NOTE**: Although both ``botsFc`` and ``botsOa`` are about gathering information about other robots, the robots will not mix those information.
E.g. if you set ``botsFc="1" botsOa="false"``, a robot will only know about the position of other robots when it is calculating the next charging station.
It will not know their position for obstacle avoidance.

## Results ##
The simulation will end if
* you hit the "terminate"-button
* all robots die
* if it has run for 50000 ticks

Afterwards the program will generate a file called ``closest_<random_seed>.txt`` which will look like this:
```
1111; 49649; 27809.2; 24.500000; 5.894913
```
The first entry represents the configuration that you have used (see above), with 1 replacing "true" and 0 replacing "false"

The second entry represents the runtime of the simulation

The third entry represents the  average time of death of the robots (the remaining robots are considered *dead* if the simulation ends)

The fourth and fifth entry represent the average amount of charges at each charging station and its standard deviation respectively.