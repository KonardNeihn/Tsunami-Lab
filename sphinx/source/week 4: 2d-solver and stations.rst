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

.. figure:: tsunami?.gif
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

.. toctree::
   :maxdepth: 2
   :caption: Contents: