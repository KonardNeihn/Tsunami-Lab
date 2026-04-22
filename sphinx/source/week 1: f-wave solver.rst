Tsunami Lab Week One
=========================

How to build the current code
---------------------------------

Run the following commands:

- nix-shell -p python3Packages.sphinx python3Packages.sphinx_rtd_theme doxygen pkgs.scons
   - **nothing works without it**
- scons
   - builds the project
- ./build/tests
   - runs the implemented tests *unclear*

To generate the Documentation in Sphinx:

- run: "make html" in the sphinx folder
- this generates html-files in sphinx>build>html

Project Report
-----------------

Creating the GitLab Repository
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Using the University URZ-ID and corresponding passwords we can access and host our code on `Gitlab Uni Jena <https://git.uni-jena.de/>`_.
By registering all team members as owners to the Repository everyone can use it to see and contribute code.

Writing the new F-Wave calculations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Our new F-Wave solver functions similarly to the Roe Riemann solver we were given as reference, therefore we can reuse the *waveSpeeds*-function entirely.
The F-Wave calculations however replace waveStrength in favor of calculating flux directly (instead of calculating the water height and then multiplying by speed to get flux). 
We achieve this by calculating the two flux-vectors for both of our Water columns via the formula given in `1.1 <https://scalable.uni-jena.de/opt/tsunami/chapters/assignment_1.html#equation-eq-swe1d>`_.
We then subtract them from eachother to receive the respective jumps in flux or Δ⁢𝑓, that we need later.
The third function for the F-Wave solver is also similar to the one from the Roe Riemann solver, in that it calls the other two functions declared before (waveSpeed and waveFlux) and and it then 
calculate eigenvector α[2] in as in αL = (matrix inversion prefix) * (sR * Δf(h) - 1 * Δf(hu)) and αR =  (matrix inversion prefix) * (-sL * Δf(h) + 1 * Δf(hu)), by first calculating the prefix consisting of
one divided by the determinant of the inversed matrix (ad-bc for 2x2 matrices). The two α's are declared as α1 and α2 in the notes of this task and are equivalent to αL and αR respectively.
Since Δf is the sum of both waves we can now decomposit it into those two waves using the formula Δf(h) = αL + αR (top part of the vectors) and Δf(hu) = αL * sL + αR * sR (bottom part of the vector).
Finally we calculate the netUpdates similarly to the Roe Riemann solver by first initializing the values with zero and then checking if the wave speeds are negative or not 
(this determines which way, or more precisely, to which column the water flows) and then adding the waves corresponding values to the correct columns iteratively with the for-loop.


Implementing tests for the new F-Wave calculations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Due to the Roe solver and f wave solver should calculate the same derivation of speeds we can reuse the test from Roe for f_wave with identical iputs.
For the flux computation, we verify that the f-wave solver correctly calculates the flux difference between the left and right states. The test uses simple input values where the expected flux difference can be derived manually.
For the net-updates, we test both a supersonic case and a steady-state case. In the supersonic case, we check that updates occur only on one side, while the other side remains zero. 
In the steady-state case with identical inputs, we ensure that the net updates vanish up to machine precision.



.. toctree::
   :maxdepth: 2
   :caption: Contents:

