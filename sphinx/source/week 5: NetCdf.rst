Tsunami Lab Week 5
=======================

Project Report
-----------------

Recording timesteps with NetCdf (instead of making more .csv files)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This weeks tasks involve NetCdf and its implementation as a replacement for .csv-files in regards to recording the tsunami simulation.
The main difference between both variations is the improved efficiency of NetCdf over .csv by simply writing all timesteps into the same file 
and therefore never recording unchanging values multiple times (such as: x-location, y-location and bathymetry). 
It is a nice bonus to also not clutter the output folder with as many files as there are tiemsteps aswell, since that is especially annoing when
there were many timesteps.

This weeks task also specifies that we should follow the COARDS convention regarding these new NetCdf output files. 
To achieve this we simply order our output data correctly (first time, then y and then x) and define global attributes, like titles.
The notes for this task also provide helpful advice for handling the new circumstances.
We do not record ghost cells and use the keyword *since* to identify the time dimension in: "seconds since the earthquake event".
Time is the most interesting recorded dimension, since it is unlimited as to allow more timesteps to be appended onto the file.
All values regarding the cells need to be written initially, but the further timesteps only need the simulation time, the water height and the two types of directional momentum.

Currently both recording variations are in use for testing purposes, but the recording via .csv-files can be easily commented out.


Input for NetCdf
~~~~~~~~~~~~~~~~~

To get the input from the NetCdf files, we implemented a `NetCdfReader` class, which can read a single variable from a NetCdf file. This data is then returned as a vector.
The complete data from a NetCdf file can be used by first reading x and y values to know the dimensions if the data, so that it can be correctly converted into the dimensions if the simulation grid.
Then the z variable (bathymetry or water displacement) can be read. Since the data is stored in a 1d vector it has to be converted into a 2d vector. Now it can be easily used inside `getHeight` and `getBathymetry`.

Checking the correctness of the file based input system two new setups were implemented. The ArtificialSetup2d is a simple setup with a flat bathymetry and a basic wave like water displacement. 
The TsunamiEvent2d setup reads a bathymetry and a water displacement from NetCdf files. The pathes to these files can be set in the `TsunamiEvent2d` constructor.  

For both setups the bathymetry is constantly at -100. For the artificial setup the water displacement varies from 160 to 161 and for the tsunami event setup it varies from 100 to 101.
The differens can probably be changed by adjusting the paramteres of the artifical setup.


.. toctree::
   :maxdepth: 2
   :caption: Contents: