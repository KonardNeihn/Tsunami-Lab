/**
 * @author 
 *
 * @section DESCRIPTION
 * One-dimensional dam break problem.
 **/
#include "TsunamiEvent2d.h"
#include <algorithm>
#include <math.h>


tsunami_lab::setups::TsunamiEvent2d::TsunamiEvent2d(std::string path) {
    m_1dBathymetry = tsunami_lab::io::NetCdf( path, 1, 1, 1, 1, nullptr ).read( path, "z" )[0];

    // build 2d bathymetry structure from 1d bathymetry
    for (size_t i = 0; i < m_1dBathymetry.size(); i++) {
        std::vector<t_real> l_row;
        for (size_t j = 0; j < m_1dBathymetry.size(); j++) {
            l_row.push_back(m_1dBathymetry[i]);
        }
        m_2dBathymetry.push_back(l_row);
    }
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getHeight( t_real i_x,
                                                                t_real i_y ) const {
    if (m_bIn < 0) {
        max(-m_bIn, m_roh);
    } else {
        return 0;
    }
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getMomentumX( t_real,
                                                                   t_real ) const {
  return 0;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getMomentumY( t_real,
                                                                   t_real ) const {
  return 0;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getBathymetry( t_real i_x,
                                                                            t_real i_y ) const {
    return m_2dBathymetry[ (int) i_x ][ (int) i_y ];
}