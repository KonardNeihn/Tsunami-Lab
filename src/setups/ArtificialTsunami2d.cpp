/**
 * @author 
 *
 * @section DESCRIPTION
 * One-dimensional dam break problem.
 **/
#include "ArtificialTsunami2d.h"
#include <algorithm>
#include <math.h>
#include <iostream>


tsunami_lab::setups::ArtificialTsunami2d::ArtificialTsunami2d(t_real i_bIn,
                    t_real i_roh) {

    m_bIn = i_bIn;
    m_roh = i_roh;
}

tsunami_lab::t_real tsunami_lab::setups::ArtificialTsunami2d::getHeight( t_real i_x, t_real i_y ) const {

    t_real l_f = std::sin((i_x / 500) + 1) * 3.141592653589793;
    t_real l_g = -((i_y / 500)*(i_y / 500)) + 1;
    
    t_real l_d = 5 * l_f * l_g;

    if (m_bIn < 0) {
        std::cout << "Height: " << -std::min(m_bIn, -m_roh) + l_d << std::endl;
        return std::max(-m_bIn, m_roh) + l_d;
    } else {
        return 0 + l_d;
    }
}

tsunami_lab::t_real tsunami_lab::setups::ArtificialTsunami2d::getMomentumX( t_real,
                                                                   t_real ) const {
  return 0;
}

tsunami_lab::t_real tsunami_lab::setups::ArtificialTsunami2d::getMomentumY( t_real,
                                                                   t_real ) const {
  return 0;
}

tsunami_lab::t_real tsunami_lab::setups::ArtificialTsunami2d::getBathymetry( t_real i_x,
                                                                            t_real i_y ) const {
    (void)i_x;
    (void)i_y;
    return -100;
}