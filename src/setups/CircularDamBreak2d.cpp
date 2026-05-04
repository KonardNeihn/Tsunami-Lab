/**
 * @section DESCRIPTION
 * Two-dimensional circular dam break setup.
 **/
#include "CircularDamBreak2d.h"

tsunami_lab::t_real tsunami_lab::setups::CircularDamBreak2d::getHeight( t_real i_x,
                                                                         t_real i_y ) const {
  if( std::sqrt( i_x * i_x + i_y * i_y ) < 10 )
    return 10;
  return 5;
}
