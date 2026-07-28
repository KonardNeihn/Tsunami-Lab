======================
Tsunami Lab FINAL
======================


Key Fixes
=========

Net update output was not reset
---------------------------------

The solver's ``netUpdates`` function did not zero its output arrays
at the start, so stale values from a previous call could leak into
the next one. This was the single largest source of the "artifacts
appearing out of nowhere" behavior seen early on, and fixing it
cleaned up the wave field dramatically.

Wave speed estimate at steep depth jumps
------------------------------------------

The wave speed estimate (Roe average) alone does not always bound
the true wave speeds at a sharp depth transition, such as a thin
coastal cell sitting right next to a deep ocean trench. This let
water columns go slightly negative during a time step. Bounding the Roe estimate against
each cell's own wave speed fixed this.

Coarse/fine coupling: mixed land/water cells
-----------------------------------------------

The most persistent and subtle bug: whenever a coarse cell straddled
both land and water, several places in the code averaged sea-level
(``eta = h + b``) across *all* sub-cells, including dry land ones.
Since a dry cell's "sea level" is really just its land elevation,
mixing it in artificially raised the water level of the coarse cell
effectively injecting energy out of nothing at every coastline and
small island.

The fix, applied consistently in the three places where coarse and
fine grids exchange data (restricting fine data up to the coarse
grid, interpolating coarse data down to the fine grid's ghost cells,
and the one-time initial sync between the two), is to average sea
level **only over the wet sub-cells**, and to treat a fully dry
coarse cell as a wall (zero height and momentum) rather than
averaging it at all.

Velocity safety net (Froude cap)
------------------------------------

As an additional safeguard, the velocity reconstructed for a fine
grid's ghost cells is capped relative to the local shallow-water
wave speed (``sqrt(g * h)``). This prevents a coarse-grid velocity
from producing an unphysically fast current if it ever gets applied
to a very thin local water column near the coast.

Limitations
==================

- **Staircase coastlines**: because the grid is Cartesian and waves
  are reflected axis-by-axis, a diagonal or curved coastline is
  approximated as a "staircase" of grid-aligned wall segments. This
  is a resolution limitation of the method rather than a bug.
- **No energy dissipation**: the model has no bottom friction or
  wave-breaking term, so coastal reflections are perfectly elastic.
  This is intentional, not a missing feature.

Visualisation
===================

.. raw:: html

   <video width="740" height="700" controls>
     <source src="_static/final.mp4" type="video/mp4">
   </video>

   ./build/tsunami_lab -S ChileEvent2d -n 200 -t 4000



When the wave propagates towards the ocean the different cell sizes can be observered. 
The red pixels under the land are only a visual bug which come from the visualisation of ``h + b``.


.. toctree::
   :maxdepth: 2
   :caption: Contents:
