Tsunami Lab Week 8
=======================

Project Report
-----------------

Draco-Cluster and Performance measures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Setting up the Draco-Cluster  was more complicated than first anticipated. First we needed to clone the Github and add the input-data from chapter 6.
The next part was installing scons with the tools/python/3.8 module. Next we needed to download PugiXml-support, since it was not installed on the cluster.
Then we needed to compile and archive it for the linker. Now we needed to expand the SConstruct file to actually link it correctly. And lastly
we neded a setup environment that would load all required modules and set the environment variables for Scons.

After all of that the code can finally be compiled on the cluster itself.

To test and measure the time on both the cluster and the elaine-pc we added time measurements around the time stepping loop and additionaly 
measure the time the solver takes in each loop to add these times up. If we now compare the two recorded times, we can not only derive the percentage
of time that the solver takes in comparison to the entire simulation, we can also easily calculate the setup time and I/O overheads.

The measurements for the Chile-setup (for comparison ./build/tsunami_lab -S ChileEvent2d -n 12000 -t 90 ) are
   - 121 368 000 cells (12000 * 10114)
   - time: 90s (176 time steps)
   - number of total cell updates: 21.360.768.000 (121368000 * 176)

For the elaine-pc we get the following measurements:
   - entire loop time: 600.438 seconds
   - solver-time: 320.129 seconds
Which gives us:
   - about 280 seconds of overhead and I/O time 
   - 53.316% of time is solver-time
   - ~1.82 seconds per time step
   - ~1.5 E-8 seconds (15 nanoseconds) per cell update

For the draco cluster we get:
   - entire loop time:  seconds
   - solver-time:  seconds
Which gives us:
   - about  seconds of overhead and I/O time 
   - % of time is solver-time
   - ~ seconds per time step
   - ~ E-8 seconds (15 nanoseconds) per cell update

With these measurements it seems like the elaine-pc is much faster which probably has to do with the fact that the simulation itself is not really
parallelized yet and therefore doesn't really benefit from the large amount of cores that are the clusters strength.

Compilers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~




VTune GUI
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. toctree::
   :maxdepth: 2
   :caption: Contents: