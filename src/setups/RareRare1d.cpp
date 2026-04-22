/**
 * @section DESCRIPTION
 * One-dimensional rare-rare problem.
 **/
#include "RareRare1d.h"

tsunami_lab::setups::RareRare1d::RareRare1d( t_real i_height,
                                             t_real i_momentumLeft,
                                             t_real i_locationMiddle ) {
  m_height       = i_height;
  m_momentumLeft = i_momentumLeft;
  m_locationMiddle  = i_locationMiddle;
}

tsunami_lab::t_real tsunami_lab::setups::RareRare1d::getHeight( t_real,
                                                                t_real ) const {
  // height is identical on both sides of the impact
  return m_height;
}

tsunami_lab::t_real tsunami_lab::setups::RareRare1d::getMomentumX( t_real i_x,
                                                                   t_real      ) const {
  // left of the impact: water moves left, meaning negative momentum
  // right of the impact: water moves right, meaning positive momentum
  if( i_x <= m_locationMiddle ) {  
    return -m_momentumLeft;
  }
  else {
    return m_momentumLeft;
  }
}

tsunami_lab::t_real tsunami_lab::setups::RareRare1d::getMomentumY( t_real,
                                                                   t_real ) const {
  // 1d problem: no momentum in y-direction
  return 0;
}
