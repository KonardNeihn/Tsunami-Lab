/**
 * 
 * @section DESCRIPTION
 * 2D Tsunami event
 **/
#ifndef TSUNAMI_LAB_SETUPS_CHILE_EVENT_2D_H
#define TSUNAMI_LAB_SETUPS_CHILE_EVENT_2D_H

#include "Setup.h"
#include <string>
#include <vector>
#include "../io/NetCdfReader.h"
#include "../config/Config.h"

namespace tsunami_lab {
  namespace setups {
    class ChileEvent2d;
  }
}

/**
 * 1d dam break setup.
 **/
class tsunami_lab::setups::ChileEvent2d: public Setup {
  private:
    Config& g_config;
    
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
    ChileEvent2d(Config& config, std::string pathBathymetry, std::string pathDisplacement);

    /**
     * Gets the water height at a given point.
     *
     * @param i_x x-coordinate of the queried point.
     * @return height at the given point. 
     **/
    t_real getHeight( t_real i_x,
                      t_real i_y     ) const override;

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

    
    t_real getBathymetry( t_real i_x,
                          t_real i_y ) const override; 

    t_real getDomainWidth() const;

    t_real getDomainLength() const;
};

#endif