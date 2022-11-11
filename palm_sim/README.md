# `palm_sim`

## Setup

This component is a python proxy for simulating a given environment of our tracking system with the built-in crazyswarm simulator. 

Before you can run this, you have to be in the `crazyswarm` anaconda environment (see the crazyswarm documentation for more details), and we must specify the python directory of the `pycrazyswarm` module. If this is installed in the root directory of your use (as we recommend), then the following command will update the `PYTHONPATH` accordingly:

```
export PYTHONPATH="${PYTHONPATH}:/home/[YOUR_USERNAME]/crazyswarm/ros_ws/src/crazyswarm/scripts"
```

You might also want to add this to your `.bashrc`.

To run the simulation, run the following command:

```
cd src # Must be in the src subdirectory for the crazyswarm config pathing to work
python palm_sim.py --sim --vis vispy 
```

## Communication

This simulator proxy communicates with `palm_core` over a socket connection, with messages as follows:

1. From `palm_core -> palm_sim`, messages are currently strings of 3 comma deliminated floats per robot, with robots deliminated by `|` (pipe).
