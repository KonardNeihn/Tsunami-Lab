/**
 * @author 
 *
 * @section DESCRIPTION
 * One-dimensional dam break with bathymetry.
 **/

#ifndef TSUNAMI_LAB_SETUPS_HYDRAULICJUMP_1D_H
#define TSUNAMI_LAB_SETUPS_HYDRAULICJUMP_1D_H

#include "Setup.h"

namespace tsunami_lab {
  namespace setups {
    class HydraulicJump1d;
  }
}

// to test subcritical flow and supercritical flow
class tsunami_lab::setups::HydraulicJump1d: public Setup {
    private:
        t_real m_bObstacle = 0; // position of the obstacle will be hardcoded, because it is same for both cases
        t_real m_bBase = 0;
        t_real m_momentum = 0;
    public:

    /**
     * Constructor.
     * @param i_obstacle height of the obstacle.
     * @param i_base base height of the bathymetry.
     * @param i_momentum momentum of the water.
     **/
     
    HydraulicJump1d(t_real i_obstacle,
                 t_real i_base,
                 t_real i_momentum
                 );

    /**
     * Gets the water height at a given point.
     *
     * @param i_x x-coordinate of the queried point.
     * @return height at the given point.
     **/
    t_real getHeight( t_real i_x,
                      t_real      ) const override;

    /**
     * Gets the momentum in x-direction.
     *
     * @return momentum in x-direction.
     **/
    t_real getMomentumX( t_real,
                         t_real ) const override;

    /**
     * Gets the momentum in y-direction.
     *
     * @return momentum in y-direction.
     **/
    t_real getMomentumY( t_real,
                         t_real ) const override;

    /**
    * Gets the bathymetry at a given point.
    *
    * @return bathymetry at the given point.
     *
    **/
    t_real getBathymetry( t_real i_x,
                          t_real      ) const override;
};

#endif