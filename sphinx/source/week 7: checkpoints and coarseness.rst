Tsunami Lab Week 7
=======================

Project Report
-----------------

Implementing Simulation Checkpoints
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Creating a Checkpoint:

A checkpoint saves the state of simulation at a specific simulation time. It saves heights, momenta and bathymetry as 2D variables and simulation time, end time, domain start, width and number of cells inside
a netCDF file. It uses two new functions `createCheckpoint` and `write2DVariable` from a new class `NetCdfCheckpoint`. The state of the simulation is saved every couple of simulation time, specified by the variable `l_checkpointRate`
inside `main.cpp`. By default the checkpoint is saved to `solutions/checkpoint.nc`. The checkpoint only saves the latest state and doesnt create multiple checkpoint files.


Loading a Checkpoint:

To continue a simulation from a checkpoint, the user needs to run a new checkpoint setup `Checkpoint2d`. This setup reads the checkpoint file a initializes all saved parameter with the help of `readCheckpointAndSetParameters` and `read2DVariable` functions.



Implementing Coarseness
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. toctree::
   :maxdepth: 2
   :caption: Contents: