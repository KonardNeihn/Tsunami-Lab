/**
 * 
 * @section DESCRIPTION
 * 2D Tsunami event
 **/
#ifndef TSUNAMI_LAB_SETUPS_TOHOKU_EVENT_2D_H
#define TSUNAMI_LAB_SETUPS_TOHOKU_EVENT_2D_H

#include "Setup.h"
#include <string>
#include <vector>
#include "../io/NetCdfReader.h"

namespace tsunami_lab {
  namespace setups {
    class TohokuEvent2d;
  }
}

/**
 * 1d dam break setup.
 **/
class tsunami_lab::setups::TohokuEvent2d: public Setup {
  private:
    // amount of cells used for the simulation
    t_idx m_nx;
    t_idx m_ny;

    std::vector<t_real> m_x;
    std::vector<t_real> m_y;
    std::vector<t_real> m_xd;
    std::vector<t_real> m_yd;

    std::vector<t_real> m_1dBathymetry;
    std::vector<std::vector<t_real>> m_2dBathymetry;

    std::vector<t_real> m_1dDisplacement;
    std::vector<std::vector<t_real>> m_2dDisplacement;

  public:
    /**
     * Constructor.
     **/
    TohokuEvent2d(std::string pathBathymetry, std::string pathDisplacement, t_idx i_nx, t_idx i_ny);

    /**
     * Gets the water height at a given point.
     *
     * @param i_x x-coordinate of the queried point.
     * @return height at the given point. 
     **/
    t_real getHeight( t_real i_x,
                      t_real i_y     ) const;

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