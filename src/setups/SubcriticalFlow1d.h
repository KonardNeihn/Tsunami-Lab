/**
 * @section DESCRIPTION
 * One-dimensional subcritical flow over a bump.
 * dhows the effect of bathymetry on wave propagation.
 **/
#ifndef TSUNAMI_LAB_SETUPS_SUBCRITICAL_FLOW_1D_H
#define TSUNAMI_LAB_SETUPS_SUBCRITICAL_FLOW_1D_H

#include "Setup.h"
#include <cmath>

namespace tsunami_lab {
  namespace setups {
    class SubcriticalFlow1d;
  }
}

class tsunami_lab::setups::SubcriticalFlow1d: public Setup {
  private:
    //! x-coordinate of the bump centre
    t_real m_bumpCenter = 0;

    //! height of the bump at its peak
    t_real m_bumpHeight = 0;

    //! width (spread) of the bump
    t_real m_bumpWidth = 0;

    //! flat water surface elevation (h + b = const)
    t_real m_waterSurface = 0;

    //! constant momentum driving flow left to right
    t_real m_momentum = 0;

  public:
    /**
     * Constructor.
     *
     * @param i_bumpCenter   x-coordinate of the bump centre.
     * @param i_bumpHeight   maximum height of the bump.
     * @param i_bumpWidth    spread of the bump (Gauss).
     * @param i_waterSurface flat water surface elevation (must exceed bump height).
     * @param i_momentum     constant momentum in x-direction.
     **/
    SubcriticalFlow1d( t_real i_bumpCenter,
                       t_real i_bumpHeight,
                       t_real i_bumpWidth,
                       t_real i_waterSurface,
                       t_real i_momentum );

    /**
     * Gets the bathymetry at a given point.
     * Returns a smooth Gaussian bump centred at m_bumpCenter.
     *
     * @param i_x x-coordinate of the queried point.
     * @return bathymetry at the given point.
     **/
    t_real getBathymetry( t_real i_x,
                          t_real      ) const;

    /**
     * Gets the water height at a given point.
     * Calculated from surface minus bathymetry: h = waterSurface - b(x).
     * Set to zero in case of negative depths.
     *
     * @param i_x x-coordinate of the queried point.
     * @return water height at the given point.
     **/
    t_real getHeight( t_real i_x,
                      t_real      ) const;

    /**
     * Gets the momentum in x-direction.
     * Returns a constant value everywhere (steady inflow).
     *
     * @return momentum in x-direction.
     **/
    t_real getMomentumX( t_real,
                         t_real ) const;

    /**
     * Gets the momentum in y-direction.
     * Zero for this 1d problem
     *
     * @return momentum in y-direction.
     **/
    t_real getMomentumY( t_real,
                         t_real ) const;
};

#endif
