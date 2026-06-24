Tsunami Lab Week 10
========================

Wave Propagation on an adaptive Grid
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``WavePropagationAdaptiveGrid2d`` class implements Adaptive Mesh Refinement to optimize simulation performance by focusing resolution where it is needed most.
For the Refinement is given once at the start at the simulation by the new setup method ``getResolution`` which return the resolution of a cell at a specific location.
The default value is ``1`` so that old simualtions run as befor the new Wave Propagation.


How it works
.................................

Both the coarse and fine grids store the same essential physical state variables, including the water height, momentum in both directions, and the bathymetry. While both use this identical data structure, the fine grid holds these values at a higher spatial resolution for the cell it represents.
``timeStep`` updates the coarse grid. If it has to update fine cells, it calculates them with ``WavePropagation2d`` befor proceeding with the coarse grid.
To handle bondaries properly, synchronizing the fine grid's ghost cells with the coarse grid's state before each sub-step, ensurs that the transition between different resolutions remains physically consistent and stable.



.. toctree::
   :maxdepth: 2
   :caption: Contents: