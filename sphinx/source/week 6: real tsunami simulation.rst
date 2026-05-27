Tsunami Lab Week 4
=======================

Project Report
-----------------

Implementing the 2008 magnitude 8.8 Chile event
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A visualisation of the input data:

..figure:: chile2010.png
   :width: 600px

    Chile event visualisation of the input data with 250m resolution.

.. figure:: 250x250_chile.gif
   :width: 600px

    Chile event visualisation with 250m resolution on a 250x250 grid.

The simulation had to run for: 0.580082 (time in ParaView)
before the first waves left the computational domain.

The computational demands are: 
    - number of required cells: 250 x 250 = 62500
    - number of cell updates: 25 x 62500 = 1562500 cell updates

Implementing the 2011 magnitude 8.8 Tohoku event
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. figure:: 100x100_250_tohoku.gif
   :width: 600px

    Tohoku event visualisation with 250m resolution on a 100x100 grid.


.. figure:: 250_tohoku.gif
   :width: 600px

    Tohoku event visualisation with 250m resolution on a 500x500 grid.

The 50m resolution version didnt work. After a short time of running the simulation, the consol outputed 'Killed'. Probably the simulation ran out of memory, since the 50m version has 100 times more cells than the 250m version.

The simulation had to run for: 0.406338
before the first waves left the computational domain. (Top Edge)

The computational demands are: 
    - number of required cells: 500 x 500 = 250000
    - number of cell updates: 25 time_steps x 250000 cells = 6250000 cell updates

The earthquake took place at 14:46 JST (UTC 05:46) around 67 kilometers from the nearest point on Japan's coastline, 
and initial estimates indicated the tsunami would have taken about 30 minutes to reach the areas first affected, 
and then areas farther north and south based on the geography of the coastline. 
Sõma, a coastal city in Fukushima Prefecture, experienced the devastating impacts and the Japan Meteorology Agency (JMA)
published their observations, including: *15:50 JST Sōma 7.3 meters or higher* (UTC 6:50). 
Tohoku University, in their paper "Tsunami arrival time characteristics of the 2011 East Japan Tsunami obtained from eyewitness accounts, evidence and numerical simulation"
mentions arrival time, in central Sõma, of 69 minutes after the Event. This matches the data of the JMA with only a few minutes difference.

To calculate the arrival time we have to consider the following facts: the distance from the epicentre to the coast
is **198 km** and the wave speed there is: **λ = √(9.81 × 1990) ≈ 140 m/s**, since the epicentre is at a depth of 1990 m.
The rule of thumb calculation leads to 198000/140 s which is about 23.5 minutes (1414 seconds). 
This is significantly faster than the real recorded arrival time, mostly because it does not take the slowing effect of
the coastal bathymetry into consideration and assumes a constant depth of 1990 meters. 
If we instead calculate a mean value for depth and speed we get λ = √(9.81 × 620) ≈ 78 m/s and therefore 42,3 minutes
which is much better, but still decently far off of the recorded 69 minutes.




.. toctree::
   :maxdepth: 2
   :caption: Contents: