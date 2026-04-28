Tsunami Lab Week Three
=======================

Project Report
-----------------

Solver Bathymetry 
~~~~~~~~~~~~~~~~~~~~~

First we implement the basic support for bathymetry by following the given formulas and adding them into the f-wave solver.
The change can be directly included in the flux-calculation (3.1.1) and is determined by Δ⁢𝑥⁢Ψ𝑖−1/2(3.1.2).
Bathymetry values also need to be included for ghost-cells in the wave-propagation patch, along with further changes 
to implement the reflecting bevavior for task 3.2.

To illustrate the effects of the bathymetry, in this example we can see the bathymetry at work.
The floor is not displayed but the sharp, stable fall in water height (5m, y-axis) at 7m (x-axis) is clearly representative of the same difference in floor height.

.. figure:: Bathymetry_3_1_2.gif
   :alt: Bathymetry example
   :width: 600px

Bathymetry example with dam at 5m and change in floor height at 7m.

Another example would be a simple subcritical flow with Water equal to the height of the floor bump piled on top of it.
Since the water is affected by the same momentum, the resulting waves have different speeds (positive momentum means the wave going left is slower)
and the scenario reaches an equilibrium with water above the raised floor being lower but faster.

.. figure:: Bathymetry_3_1_3.gif
   :alt: Bathymetry example 2
   :width: 600px

Bathymetry example with a bump (gaussian) at the middle with a maximium floor height of 0-8m and a width of 0.5m.

Boundary Reflections
~~~~~~~~~~~~~~~~~~~~~~~

The basic idea of the reflecting boundary cells is very simple: if a boundary cell is wet, the outflow should behave like it already was.
If it is dry, it should reflect the wave exactly like in a shock-shock scenario.
The implementation is abit more difficult, since we now have to track the dry/wet or outflow/reflecting states of both boundary cells individually.


Hydraulic Jumps - Subcritical to Supercritical Flow
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


.. toctree::
   :maxdepth: 2
   :caption: Contents: