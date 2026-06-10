Tsunami Lab Week 8
=======================

Project Report
-----------------

Draco-Cluster and Performance measures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Setting up the Draco-Cluster  was more complicated than first anticipated. First we needed to clone the Github and add the input-data from chapter 6.
The next part was installing scons with the tools/python/3.8 module. Next we needed to download PugiXml-support, since it was not installed on the cluster.
Then we needed to compile and archive it for the linker. Now we needed to expand the SConstruct file to actually link it correctly. And lastly
we neded a setup environment that would load all required modules and set the environment variables for Scons. Also we need to disable version-checking for hdf5, since it throws warnings.

After all of that the code can finally be compiled on the cluster itself.

To test and measure the time on both the cluster and the elaine-pc we added time measurements around the time stepping loop and additionaly 
measure the time the solver takes in each loop to add these times up. If we now compare the two recorded times, we can not only derive the percentage
of time that the solver takes in comparison to the entire simulation, we can also easily calculate the setup time and I/O overheads.

The measurements for the Chile-setup (for comparison ./build/tsunami_lab -S ChileEvent2d -n 8000 -t 90 ) are:
   - 53 936 000 cells (8000 * 6742)
   - time: 90s (118 time steps)
   - number of total cell updates: 6.364.448.000 (53936000 * 118)

For the elaine-pc we get the following measurements:
   - entire loop time: 174.867 seconds
   - solver-time: 94.7481 seconds
Which gives us:
   - about 80 seconds of overhead and I/O time 
   - 54.1831% of time is solver-time
      - ~0.80295 seconds per time step
      - ~1.5 E-8 seconds (15 nanoseconds) per cell update

For the draco cluster we get (live testing with salloc --partition=short):
   - entire loop time: 451.856 seconds
   - solver-time: 286.352 seconds
Which gives us:
   - about 165.5 seconds of overhead and I/O time 
   - 63.3724% of time is solver-time
      - ~2.43 seconds per time step
      - ~4.5 E-8 seconds (45 nanoseconds) per cell update

With these measurements, the elaine-pc is much faster which probably has to do with the fact that the simulation itself is not really
parallelized yet and therefore doesn't really benefit from the large amount of cores that are the clusters strength. It takes almost three times
as long on the cluster node.

Compilers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~





VTune GUI
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. toctree::
   :maxdepth: 2
   :caption: Contents: