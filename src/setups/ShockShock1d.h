/**
 * @section DESCRIPTION
 * One-dimensional shock-shock problem.
 **/
#ifndef TSUNAMI_LAB_SETUPS_SHOCK_SHOCK_1D_H
#define TSUNAMI_LAB_SETUPS_SHOCK_SHOCK_1D_H

#include "Setup.h"

namespace tsunami_lab {
  namespace setups {
    class ShockShock1d;
  }
}

/**
 * 1d shock-shock setup.
 * Two streams of water move toward each other and collide at a certain x-coordinate.
 * Left state:  hu > 0 (moves right)
 * Right state: hu < 0 (moves left), mirrored from left state.
 **/
class tsunami_lab::setups::ShockShock1d: public Setup {
  private:
    //! water height (same on both sides)
    t_real m_height = 0;

    //! momentum (left state)
    t_real m_momentumLeft = 0;

    //! location of the waves impact
    t_real m_locationImpact = 0;

  public:
    /**
     * Constructor.
     *
     * @param i_height water height on both sides.
     * @param i_momentumLeft momentum on left side (hu > 0), no need for momentumRight we can just invert this one.
     * @param i_locationImpact location of the impact of both waves.
     **/
    ShockShock1d( t_real i_height,
                  t_real i_momentumLeft,
                  t_real i_locationImpact );

    /**
     * Gets the water height at a given point.
     * Returns the same height on both sides.
     *
     * @param i_x x-coordinate of the queried point.
     * @return height at the given point.
     **/
    t_real getHeight( t_real i_x,
                      t_real      ) const;

    /**
     * Gets the momentum in x-direction.
     * Returns +momentumLeft for x <= i_locationImpact, -momentumLeft for x > i_locationImpact.
     *
     * @param i_x x-coordinate of the queried point.
     * @return momentum in x-direction.
     **/
    t_real getMomentumX( t_real i_x,
                         t_real      ) const;

    /**
     * Gets the momentum in y-direction.
     * We don't have a second Dimension for this 1d problem.
     *
     * @return momentum in y-direction.
     **/
    t_real getMomentumY( t_real,
                         t_real ) const;
};

#endif
