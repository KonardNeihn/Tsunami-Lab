Tsunami Lab Week 9
=======================

Project Report
-----------------

Parallelization with OpenMP
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


Implementing the code on Grace-Cluster
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


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
