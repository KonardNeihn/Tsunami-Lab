/**
 * @section DESCRIPTION
 * Two-dimensional circular dam break setup.
 * Domain: [-50, 50]^2
 * Initial condition:
 *   [h, hu, hv]^T = [10, 0, 0]^T  if sqrt(x^2 + y^2) < 10
 *   [h, hu, hv]^T = [5,  0, 0]^T  else
 * Flat bathymetry, all boundaries outflow.
 **/
#ifndef TSUNAMI_LAB_SETUPS_CIRCULAR_DAM_BREAK_2D
#define TSUNAMI_LAB_SETUPS_CIRCULAR_DAM_BREAK_2D

#include "Setup.h"
#include <cmath>

namespace tsunami_lab {
  namespace setups {
    class CircularDamBreak2d;
  }
}

class tsunami_lab::setups::CircularDamBreak2d: public Setup {
  public:
    /**
     * No constructor arguments needed — all parameters are fixed by the
     * problem specification.
     **/
    CircularDamBreak2d() = default;

    /**
     * Gets the water height at a given point.
     * Returns 10 inside the dam (radius < 10), 5 outside.
     *
     * @param i_x x-coordinate of the query point.
     * @param i_y y-coordinate of the query point.
     * @return water height.
     **/
    t_real getHeight( t_real i_x,
                      t_real i_y ) const;

    /**
     * Gets the momentum in x-direction (always 0).
     **/
    t_real getMomentumX( t_real,
                         t_real ) const { return 0; }

    /**
     * Gets the momentum in y-direction (always 0).
     **/
    t_real getMomentumY( t_real,
                         t_real ) const { return 0; }

    /**
     * Gets the bathymetry (always 0 — flat domain).
     **/
    t_real getBathymetry( t_real i_x,
                          t_real i_y ) const; 

    // All four boundaries are outflow — no overrides needed, since the base class defaults to false for all of them.
};

#endif