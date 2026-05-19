/**
 * 
 * @section DESCRIPTION
 * 2D Tsunami event
 **/
#ifndef TSUNAMI_LAB_SETUPS_TSUNAMI_EVENT_2D_H
#define TSUNAMI_LAB_SETUPS_TSUNAMI_EVENT_2D_H

#include "Setup.h"

namespace tsunami_lab {
  namespace setups {
    class TsunamiEvent2d;
  }
}

/**
 * 1d dam break setup.
 **/
class tsunami_lab::setups::TsunamiEvent2d: public Setup {
  private:

  public:
    /**
     * Constructor.
     **/
    TsunamiEvent2d(std::string path);

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