Tsunami Lab Week 4
=======================

Project Report
-----------------

Stations
~~~~~~~~~~~~~~~~~~~~~

Each station has a position represented by x and y coordinates as well as a name to separate them easily. Every station has a recording interval which is the same for all stations.
The output files will be saved inside the folder ``solutions/station_data``. The file is named after its station. The station class contains a public method ``timeStep`` handling the recording of the data.
``timeStep`` gets called in main inside the main simulation loop and checks with the help of ``l_dt`` if it should record or keep waiting until the time since the last record is high enough.
The csv files can be converted into plots the visulize the recorded data by running ``python3 station_vis.py`` in the command line. The outputed image will be safed inside ``solutions/stations_data/plots``.

.. figure:: t
   :width: 600px

Example of recorded station data. The station was placed in the center of an old RareRare1d setup.

The number of stations, their positions, the recording interval, and the location of the recorded data output folder can all be set inside ``src/io/stations.xml``.
Based on the content, instances of the station class will be created.

::
   <tsunamilab>
      <stations>
         <station name="StationA" x="5" y="0"/>
         <station name="StationB" x="2" y="0"/>
      </stations>

      <output>
         <record_intervall>0.1</record_intervall>
         <path>solutions/station_data/</path>
      </output>
   </tsunamilab>

``stations.xml`` file

2D Circular Dambreak
~~~~~~~~~~~~~~~~~~~~~

The 2D dam break was run with a circular dam with the height of 15 at the position x = 2.5 and y = 5 and a radius of 2.5, leading to the most right point of the dam being at x = 5.
This makes the 2d version is now compareable to the 1d version. Five Station were placed on the x-Axis to record the data of the 2d simulation. The solution files of DamBreak2d were
loaded into Paraview and got converted into an gif:

.. figure:: DamBreak2d.gif
   :width: 600px

   New DamBreak2d Simulation.


.. figure:: LeftDamBreak2dStation.png
   :width: 600px

   Recorded Data by a Station located on the left side of DamBreak2d.


.. figure:: RightDamBreak2dStation.png
   :width: 600px

   Recorded Data by a Station located on the right side of DamBreak2d.


.. figure:: dambreak_15_10_5_100.gif
   :width: 600px

   Old DamBreak1d Simulation.


.. toctree::
   :maxdepth: 2
   :caption: Contents: