/**
 * @section DESCRIPTION
 * One-dimensional shock-shock problem.
 **/
#include "ShockShock1d.h"

tsunami_lab::setups::ShockShock1d::ShockShock1d( t_real i_height,
                                                 t_real i_momentumLeft,
                                                 t_real i_locationImpact ) {
  // initializing transferred values
  m_height       = i_height;
  m_momentumLeft = i_momentumLeft;
  m_locationImpact  = i_locationImpact;
}

tsunami_lab::t_real tsunami_lab::setups::ShockShock1d::getHeight( t_real,
                                                                  t_real ) const {
  // height is identical on both sides of the impact
  return m_height;
}

tsunami_lab::t_real tsunami_lab::setups::ShockShock1d::getMomentumX( t_real i_x,
                                                                     t_real      ) const {
  // left of the impact: water moves right, meaning positive momentum
  // right of the impact: water moves left, meaning negative momentum
  if( i_x <= m_locationImpact ) {          // technically this means that at i_x itself the momentum is positive
    return m_momentumLeft;
  }
  else {
    return -m_momentumLeft;
  }
}

tsunami_lab::t_real tsunami_lab::setups::ShockShock1d::getMomentumY( t_real,
                                                                     t_real ) const {
  // 1d problem: no momentum in y-direction
  return 0;
}
