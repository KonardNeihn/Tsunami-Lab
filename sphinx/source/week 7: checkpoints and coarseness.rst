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

To implement a more coarse output we need a new input variable *k* to pass to the NetCdf-writer with which we can reduce the size of the output
from [m x n] to [(m/k) x (n/k)]. To achieve this we average neighbouring cells into one singular output cell by adding their values together and
dividing the result by the number of added cells. 

However there are two main Problems: 
   - the added cells need to actually be next to each other locally in the simulation grid
   - the edges of the simulation cause problems if the ratios of m/k or n/k are not "clean" numbers (something like: 500/10 x 400/10 should not cause problems (m = 500,n = 400,k = 10))

To solve these problems, we use cell blocks the size of m*k or n*k which center on (nx (and ny) + 0.5) * k * dxy. 
This allows paraView to correctly display the coarse blocks as normal cells.
At interior boundaries (when nx or ny is not divisible by k) the last block is smaller than k*k and we need to divide by the actual cell count so the average is still correct.
Since the coarseness unifies blocks of k*k we can expect k^2 less output values (for k = 4 means 16 times less values).

To use the new coarseness feature we simply need to pass a new argument *k* to the NetCdf-writer in main.cpp, which we can change as a command line argument.
Additionally the NetCdf-writer now uses compression with chunks to reduce the size of the output files.

.. toctree::
   :maxdepth: 2
   :caption: Contents: