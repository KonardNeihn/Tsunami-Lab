/**
 * @author 
 *
 * @section DESCRIPTION
 * One-dimensional rare-rare wave problem.
 **/
#ifndef TSUNAMI_LAB_SETUPS_RARE_RARE_1D_H
#define TSUNAMI_LAB_SETUPS_RARE_RARE_1D_H

#include "Setup.h"

namespace tsunami_lab {
  namespace setups {
    class RareRare1d;
  }
}

/**
 * 1d dam break setup.
 **/
class tsunami_lab::setups::RareRare1d: public Setup {
  private:
    //! height on both sides
    t_real m_height = 0;
    
    //! momentum on the left side
    t_real m_momentumLeft = 0;

    //! location of the middle, which the waves mova away from
    t_real m_locationMiddle = 0;

  public:
    /**
     * Constructor.
     *
     * @param i_heightLeft water height on the left side.
     * @param i_momentumLeft momentum on left side (hu > 0), no need for momentumRight we can just invert this one.
     * @param i_locationMiddle location (x-coordinate) of the point both waves are moving away from.
     **/
    RareRare1d( t_real i_heightLeft,
                t_real i_momentumLeft,
                t_real i_locationMiddle );

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
     * Returns -momentumLeft for x <= i_locationMiddle, +momentumLeft for x > i_locationMiddle. The inverted values of  the shock-shock problem.
     *
     * @return momentum in x-direction.
     **/
    t_real getMomentumX( t_real i_x,
                         t_real     ) const override;

    /**
     * Gets the momentum in y-direction.
     * We don't have a second Dimension for this 1d problem.
     *
     * @return momentum in y-direction.
     **/
    t_real getMomentumY( t_real,
                         t_real ) const override;

};

#endif