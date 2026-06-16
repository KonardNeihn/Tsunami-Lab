Tsunami Lab Week 9
=======================

Project Report
-----------------

Parallelization with OpenMP
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A rectangular simulation grid will always have rows and columns of equal length and therefore the necessary calculations needed 
for one individual row or column should be equal aswell. So if we allocate the same amount of rows (or columns) to each OpenMP-thread
they should take equal time to run.

Furthermore there are multiple things to consider when parallelizing the solver (wavePropagation) with OpenMP:

   - The first thing to consider for parallelization is the creation of the new buffer value array, specifically the initialization with the
    former values. Since this is simple and independent copying of values, the parallelization does not need any restrictions or further consideration.

   - The second thing to consider is the x-sweep of the actual calculation. Here we can calculate all of the rows of the simulation in parallel without 
   the threads ever interfering with each other. Therefore parallelizing the outer loop (iteration over y-values) is better.

   - The third thing to consider is the y-sweep of the calculation. Parallelizing the rows would lead to interference (race conditions) between the 
   threads, so instead we parallelize the inner loop (iteration over x-values), essentially the columns to prevent that.

Since the calculation themselves, in regards to each individual cell are always extremely similar and have no variable complexity, we can 
use *schedule(static)* to allocate the workloads for all threads (representing the cores) only once, which causes very little allocation overhead.
To dwtermine the number of cores the simulation should use, we use for example *export OMP_NUM_THREADS=72* to manually set the value to 72.



Implementing the code on Grace-Cluster
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

First there were some missing dependencies for *pugixml* used by our stations. This was easily fixed by manually re-linking the dependencies
already installed for use on the draco-cluster in SConstruct. There were also some more specific changes like manually setting library paths
that needed to be commented out or replaced, since these changes are only correct fir the draco-cluster environment.

The Nvidia Grace CPU-chip is essentially two 72-core CPUs, connected on one board. Both CPUs have their own memory caches and when a program uses 
more than 72 cores (meaning more than just one CPU) there might be NUMA-problems (Non-Uniform Memory Access). This can slow the calculations 
considerably and therefore it could be better to just use 72 cores.

With *export OMP_PROC_BIND=true* we can pin the cores and with *export OMP_PLACES=cores* we make sure that threads actually run on individual cores. 

Using the same simulation benchmark from last week to measure performance, but without checkpoints (./build/tsunami_lab -S ChileEvent2d -n 8000 -t 90) we get:

**Very inconsistent results**

   - One one single core (export OMP_NUM_THREADS=1):
      - loop-time: 217.777 seconds
      - solver-time: 123.146 seconds
      - solver percentage of time: 56.5469 %
   
   -On 10 cores:                                   with less written (after 100 instead of 25)        without writing at all
      - loop-time: 159.642 seconds                 66.6028     108.768                                28.3084
      - solver-time: 73.8167 seconds               29.2687     48.1565                                12.7496
      - solver percentage of time: 46.2388 %       43.9452     44.2745                                45.0382

   - On 72 cores
      - loop-time: 177.652 seconds   (397.737)
      - solver-time: 91.8495 seconds (279.331)
      - solver percentage of time: 51.702 %  (70.3201)
   
   - On 144 cores (did not run at all)
      - loop-time:  seconds
      - solver-time:  seconds
      - solver percentage of time:  %



Inner vs Outer Loop Parallelization
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table:: Benchmark (2000x2000, 32 Threads, 100s)
   :widths: 60 40
   :header-rows: 1

   * - Strategy
     - Runtime
   * - **Outer Loop**
     - 679.933
   * - **Inner Loop**
     - 1167.130

Parallelizing the outer loop provides better performance because entire inner loops are distributed among the available threads. When the inner loop is parallelized, its iterations have to be distributed among the threads for every iteration of the outer loop, leading to increased synchronization and scheduling overhead.

     

.. toctree::
   :maxdepth: 2
   :caption: Contents:
