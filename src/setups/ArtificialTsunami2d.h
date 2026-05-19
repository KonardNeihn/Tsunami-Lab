/**
 * 
 * @section DESCRIPTION
 * One-dimensional dam break problem.
 **/
#ifndef TSUNAMI_LAB_SETUPS_ARTIFICIAL_TSUNAMI_2D_H
#define TSUNAMI_LAB_SETUPS_ARTIFICIAL_TSUNAMI_2D_H

#include "Setup.h"

namespace tsunami_lab {
  namespace setups {
    class ArtificialTsunami2d;
  }
}

/**
 * 1d dam break setup.
 **/
class tsunami_lab::setups::ArtificialTsunami2d: public Setup {
  private:
    t_real m_bIn;
    t_real m_roh;

  public:
    /**
     * Constructor.
     **/
    ArtificialTsunami2d(t_real i_bIn,
                    t_real i_roh);

    /**
     * Gets the water height at a given point.
     *
     * @param i_x x-coordinate of the queried point.
     * @return height at the given point. 
     **/
    t_real getHeight( t_real i_x,
                      t_real      ) const;

    /**
     * Gets the momentum in x-direction.
     *
     * @return momentum in x-direction.
     **/
    t_real getMomentumX( t_real,
                         t_real ) const;

    /**
     * Gets the momentum in y-direction.
     *
     * @return momentum in y-direction.
     **/
    t_real getMomentumY( t_real,
                         t_real ) const;

    
    t_real getBathymetry( t_real i_x,
                          t_real i_y ) const; 
};

#endif