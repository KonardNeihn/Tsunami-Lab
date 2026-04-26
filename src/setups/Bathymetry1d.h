/**
 * @author 
 *
 * @section DESCRIPTION
 * One-dimensional dam break with bathymetry.
 **/

#ifndef TSUNAMI_LAB_SETUPS_BATHYMETRY_1D_H
#define TSUNAMI_LAB_SETUPS_BATHYMETRY_1D_H

#include "Setup.h"

namespace tsunami_lab {
  namespace setups {
    class Bathymetry1d;
  }
}

// to test bathymetry, we use damn break with bathymetry
class tsunami_lab::setups::Bathymetry1d::Bathymetry1d: public Setup {
    private:
        t_real m_heightLeft = 0;
        t_real m_heightRight = 0;
        t_real m_locationDam = 0;
        t_real m_bathymetryLeft = 0;
        t_real m_bathymetryRight = 0;
        t_real m_locationBathymetry = 0;
    public:

    /**
     * Constructor.
     * @param i_heightLeft water height on the left side of the dam.
     * @param i_heightRight water height on the right side of the dam.
     * @param i_locationDam location of the dam.
     * @param i_bathymetryLeft bathymetry on the left side.
     * @param i_bathymetryRight bathymetry on the right side.
     * @param i_locationBathymetry location where the bathymetry changes.
     **/
    Bathymetry1d(t_real i_heightLeft,
                 t_real i_heightRight,
                 t_real i_locationDam,
                 t_real i_bathymetryLeft,
                 t_real i_bathymetryRight,
                 t_real i_locationBathymetry);

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
};

#endif