/**
 * @author 
 *
 * @section DESCRIPTION
 * One-dimensional bathymetry problem.
 **/
#include "Bathymetry1d.h"

tsunami_lab::setups::Bathymetry1d::Bathymetry1d( t_real i_heightLeft,
                                                 t_real i_heightRight,
                                                 t_real i_locationDam,
                                                 t_real i_bathymetryLeft,
                                                 t_real i_bathymetryRight,
                                                 t_real i_locationBathymetry ) {
  m_heightLeft = i_heightLeft;
  m_heightRight = i_heightRight;
  m_locationDam = i_locationDam;
  m_bathymetryLeft = i_bathymetryLeft;
  m_bathymetryRight = i_bathymetryRight;
  m_locationBathymetry = i_locationBathymetry;
}

tsunami_lab::t_real tsunami_lab::setups::Bathymetry1d::getHeight( t_real i_x,
                                                                    t_real      ) const {
  if( i_x < m_locationDam ) {
    if(i_x < m_locationBathymetry) {
      return m_heightLeft - m_bathymetryLeft;
    }
    else {
      return m_heightLeft - m_bathymetryRight;
    }
  }
  else {
    if(i_x < m_locationBathymetry) {
      return m_heightRight - m_bathymetryLeft;
    }
    else {
      return m_heightRight - m_bathymetryRight;
    }
  }
}

tsunami_lab::t_real tsunami_lab::setups::Bathymetry1d::getMomentumX( t_real,
                                                                     t_real ) const {
  return 0;
}

tsunami_lab::t_real tsunami_lab::setups::Bathymetry1d::getMomentumY( t_real,
                                                                     t_real ) const {
  return 0;
}

tsunami_lab::t_real tsunami_lab::setups::Bathymetry1d::getBathymetry( t_real i_x,
                                                                        t_real ) const {
  if( i_x < m_locationBathymetry ) {
    return m_bathymetryLeft;
  }
  else {
    return m_bathymetryRight;
  }
}