Tsunami Lab Week 4
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


.. toctree::
   :maxdepth: 2
   :caption: Contents: