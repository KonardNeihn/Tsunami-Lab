/**
 * @author 
 *
 * @section DESCRIPTION
 * One-dimensional dam break problem.
 **/
#include "DamBreak2d.h"

tsunami_lab::setups::DamBreak1d::DamBreak2d( t_real i_heightLeft,
                t_real i_heightRight,
                t_real i_locationDamX,
                t_real i_locationDamY,
                t_real i_radiusDam ) {
  m_heightLeft = i_heightLeft;
  m_heightRight = i_heightRight;
  m_locationDamX = i_locationDamX;
  m_locationDamY = i_locationDamY;
  m_radiusDam = i_radiusDam;
}

tsunami_lab::t_real tsunami_lab::setups::DamBreak2d::getHeight( t_real i_x,
                                                                t_real i_y ) const {
  t_real dx = i_x - m_locationDamX;
  t_real dy = i_y - m_locationDamY;

  t_real distanceSquared = dx * dx + dy * dy;
  t_real radiusSquared   = m_radiusDam * m_radiusDam;

  if( distanceSquared < radiusSquared ) {
    return m_heightLeft;
  }
  else {
    return m_heightRight;
  }
}

tsunami_lab::t_real tsunami_lab::setups::DamBreak1d::getMomentumX( t_real,
                                                                   t_real ) const {
  return 0;
}

tsunami_lab::t_real tsunami_lab::setups::DamBreak1d::getMomentumY( t_real,
                                                                   t_real ) const {
  return 0;
}