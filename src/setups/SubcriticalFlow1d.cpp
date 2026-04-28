/**
 * @section DESCRIPTION
 * One-dimensional subcritical flow over a bump.
 **/
#include "SubcriticalFlow1d.h"
#include <algorithm>

tsunami_lab::setups::SubcriticalFlow1d::SubcriticalFlow1d( t_real i_bumpCenter,
                                                           t_real i_bumpHeight,
                                                           t_real i_bumpWidth,
                                                           t_real i_waterSurface,
                                                           t_real i_momentum ) {
  m_bumpCenter   = i_bumpCenter;
  m_bumpHeight   = i_bumpHeight;
  m_bumpWidth    = i_bumpWidth;
  m_waterSurface = i_waterSurface;
  m_momentum     = i_momentum;
}

tsunami_lab::t_real tsunami_lab::setups::SubcriticalFlow1d::getBathymetry( t_real i_x,
                                                                           t_real      ) const {
  // Create example bathymetry
  // Gaussian bump: b(x) = bumpHeight * exp( -(x - center)^2 / (2 * width^2) )
  // This gives a smooth hill with peak at m_bumpCenter that decays to ~0 away from the centre.
  t_real l_exponent = ( i_x - m_bumpCenter ) * ( i_x - m_bumpCenter ) / ( 2.0 * m_bumpWidth * m_bumpWidth );
  return m_bumpHeight * std::exp( -l_exponent );
}

tsunami_lab::t_real tsunami_lab::setups::SubcriticalFlow1d::getHeight( t_real i_x,
                                                                       t_real      ) const {
  // water surface is flat: h(x) + b(x) = waterSurface
  // therefore:             h(x) = waterSurface - b(x)
  // prevent below zero to prevent negative depth above a very tall bump
  t_real l_b = getBathymetry( i_x, 0 );
  return std::max( t_real(0), m_waterSurface - l_b );
}

tsunami_lab::t_real tsunami_lab::setups::SubcriticalFlow1d::getMomentumX( t_real,
                                                                          t_real ) const {
  // constant momentum everywhere — drives a steady left-to-right flow
  return m_momentum;
}

tsunami_lab::t_real tsunami_lab::setups::SubcriticalFlow1d::getMomentumY( t_real,
                                                                          t_real ) const {
  // 1d problem: no momentum in y-direction
  return 0;
}
