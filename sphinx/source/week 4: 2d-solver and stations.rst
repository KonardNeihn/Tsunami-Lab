Tsunami Lab Week 4
=======================

Project Report
-----------------

Implementing 2-Dimensional Simulation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To implement the 2-dimensional simulation, we first need to write a new patch that allows us to 
run the f-wave solver multiple times on the same data. This can be achieved by essentially storing the
states of every cell, then applying the solver on slices in x-direction and seperately storing the changes,
then running the the solver in y-direction and adding the resulting changes to the ones from the x-direction.
These changes can then be applied to the cells to create the new timestep and the simulation then continues
with the new timestep as a basis.

A large portion of the work for this task comes from implementing the needed changes into the main function and
making sure everything still works afterwards since the entire simulation always consists of atleast: 
calling the main function, using a setup, running a patch and applying a solver.

To display the new 2-D simulation, we use ParaView to apply filters to the output- *.csv-files* 
to create an animation going through the simulated timesteps. 
The specified 2-dimensional-dambreak looks like this:

.. raw:: html

   <video width="740" height="700" controls>
     <source src="_static/2ddambreak.mp4" type="video/mp4">
   </video>

Most notably the waves look unnatural, probably due to the fact that we only simulate in two directions
and therefore the diagonal waves move differently. The coloring applied to the water represents the combined
momentum in both directions, which *in my opinion* shows the movements better than just one of the two.

To showcase the bathymetry, the newer simulation contains some changes on the ground which has measurable, 
but small impacts on the overall simulation. The largest difference is in the time it takes the waves to leave
the simulated area and with what speed. Without bathymetry they all exit the simulation at the sam time due to
the area being a square, but with bathymetry that changes. It also introduces more weird momentum-areas along
the underwater cliffs in addition to the directional ones forming from the dambreak itself.

.. raw:: html

   <video width="740" height="700" controls>
     <source src="_static/2dbathymetry.mp4" type="video/mp4">
   </video>

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