/**
 * @author 
 *
 * @section DESCRIPTION
 * One-dimensional Hydraulic Jump problem.
 **/
#include "HydraulicJump1d.h"

tsunami_lab::setups::HydraulicJump1d::HydraulicJump1d( t_real i_obstacle,
                                                       t_real i_base,
                                                       t_real i_momentum ) {
  m_bObstacle = i_obstacle;
  m_bBase = i_base;
  m_momentum = i_momentum;
}
                                                 

tsunami_lab::t_real tsunami_lab::setups::HydraulicJump1d::getHeight( t_real i_x,
                                                                    t_real      ) const {
  if( i_x < 12 && i_x > 8 ) { 
    return -(m_bObstacle - 0.05 * ((i_x -10) * (i_x - 10)));
  }
  else {
    return -m_bBase;
  }
}

tsunami_lab::t_real tsunami_lab::setups::HydraulicJump1d::getMomentumX( t_real,
                                                                     t_real ) const {
  return m_momentum;
}

tsunami_lab::t_real tsunami_lab::setups::HydraulicJump1d::getMomentumY( t_real,
                                                                     t_real ) const {
  return 0;
}

tsunami_lab::t_real tsunami_lab::setups::HydraulicJump1d::getBathymetry( t_real i_x,
                                                                        t_real ) const {
  if( i_x < 12 && i_x > 8 ) { 
    return m_bObstacle - 0.05 * ((i_x -10) * (i_x - 10));
  }
  else {
    return m_bBase;
  }
}