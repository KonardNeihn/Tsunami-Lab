.. Tsunami Lab documentation master file, created by
   sphinx-quickstart on Fri Apr 10 15:36:32 2026.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Tsunami Lab documentation
=========================

Add your content using ``reStructuredText`` syntax. See the
`reStructuredText <https://www.sphinx-doc.org/en/master/usage/restructuredtext/index.html>`_
documentation for details.

How to build the current code
---------------------------------

Run the following commands:

- Old setup: nix-shell -p python3Packages.sphinx python3Packages.sphinx_rtd_theme doxygen pkgs.scons
- **New setup**: nix-shell 
   - now uses seperate file (shell.nix) to load packages, still:  **nothing works without it**
- scons
   - builds the project
- ./build/tests
   - runs the implemented tests *unclear*

To generate the Documentation in Sphinx:

- run: "make html" in the sphinx folder
- this generates html-files in sphinx>build>html

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   week 1: f-wave solver
   week 2: wave types and dambreak
   week 3: seafloors and tsunamis
   week 4: 2d-solver and stations
   week 5: NetCdf

