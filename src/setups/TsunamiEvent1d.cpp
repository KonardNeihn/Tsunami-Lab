#include "TsunamiEvent1d.h"
#include <algorithm>   // std::max, std::min, std::lower_bound
#include <cmath>       // std::sin, M_PI
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace setup = tsunami_lab::setups;

//------------------------------------------------------------------
// Constructor: load bathymetry CSV
//------------------------------------------------------------------
setup::TsunamiEvent1d::TsunamiEvent1d( const std::string & i_bathymetryFile ) {
  std::ifstream l_file( i_bathymetryFile );
  if( !l_file.is_open() ) {
    throw std::runtime_error(
      "TsunamiEvent1d: cannot open bathymetry file: " + i_bathymetryFile
    );
  }

  std::string l_line;
  // Skip header line if present, detected by checking whether first value is a number
  if( std::getline( l_file, l_line ) ) {
    std::istringstream l_ss( l_line );
    tsunami_lab::t_real l_testVal;
    if( !( l_ss >> l_testVal ) ) {
      // First line is a header – already consumed, continue reading data
    } else {
      // if the first line was data, parse it
      std::istringstream l_data( l_line );
      tsunami_lab::t_real l_x = 0, l_b = 0;
      char l_comma;
      l_data >> l_x >> l_comma >> l_b;
      m_bathyX.push_back( l_x );
      m_bathyB.push_back( l_b );
    }
  }

  // Read remaining lines
  while( std::getline( l_file, l_line ) ) {
    if( l_line.empty() ) continue;
    std::istringstream l_ss( l_line );
    tsunami_lab::t_real l_x = 0, l_b = 0;
    char l_comma;
    // supports both comma-separated and whitespace-separated files
    if( l_line.find(',') != std::string::npos ) {
      l_ss >> l_x >> l_comma >> l_b;
    } else {
      l_ss >> l_x >> l_b;
    }
    m_bathyX.push_back( l_x );
    m_bathyB.push_back( l_b );
  }

  if( m_bathyX.empty() ) {
    throw std::runtime_error(
      "TsunamiEvent1d: no data found in bathymetry file: " + i_bathymetryFile
    );
  }
}

//------------------------------------------------------------------
// Private helpers
//------------------------------------------------------------------
tsunami_lab::t_real
setup::TsunamiEvent1d::getBathymetryRaw( tsunami_lab::t_real i_x ) const {
  // Clamp to domain edges
  if( i_x <= m_bathyX.front() ) return m_bathyB.front();
  if( i_x >= m_bathyX.back()  ) return m_bathyB.back();

  // Binary search for the interval [x_k, x_{k+1}] containing i_x
  auto l_it = std::lower_bound( m_bathyX.begin(), m_bathyX.end(), i_x );
  std::size_t l_right = static_cast<std::size_t>( l_it - m_bathyX.begin() );
  std::size_t l_left  = l_right - 1;

  // Linear interpolation
  tsunami_lab::t_real l_x0 = m_bathyX[l_left];
  tsunami_lab::t_real l_x1 = m_bathyX[l_right];
  tsunami_lab::t_real l_b0 = m_bathyB[l_left];
  tsunami_lab::t_real l_b1 = m_bathyB[l_right];

  tsunami_lab::t_real l_t = ( i_x - l_x0 ) / ( l_x1 - l_x0 );
  return l_b0 + l_t * ( l_b1 - l_b0 );
}

tsunami_lab::t_real
setup::TsunamiEvent1d::getDisplacement( tsunami_lab::t_real i_x ) {
  // d(x) = 10 * sin( (x - 175000) / 37500 * pi + pi ),  if 175000 < x < 250000
  //       = 0,                                            otherwise
  if( i_x > 175000 && i_x < 250000 ) {
    return static_cast<tsunami_lab::t_real>(
      10.0 * std::sin( ( i_x - 175000.0 ) / 37500.0 * M_PI + M_PI )
    );
  }
  return static_cast<tsunami_lab::t_real>( 0 );
}

//------------------------------------------------------------------
// Setup interface
//------------------------------------------------------------------
tsunami_lab::t_real
setup::TsunamiEvent1d::getHeight( tsunami_lab::t_real i_x,
                                   tsunami_lab::t_real      ) const {
  tsunami_lab::t_real l_bIn = getBathymetryRaw( i_x );
  if( l_bIn < 0 ) {
    // h = max( -b_in, delta )
    return std::max( -l_bIn, m_delta );
  }
  return static_cast<tsunami_lab::t_real>( 0 );
}

tsunami_lab::t_real
setup::TsunamiEvent1d::getMomentumX( tsunami_lab::t_real,
                                      tsunami_lab::t_real ) const {
  // Water is initially at rest
  return static_cast<tsunami_lab::t_real>( 0 );
}

tsunami_lab::t_real
setup::TsunamiEvent1d::getMomentumY( tsunami_lab::t_real,
                                      tsunami_lab::t_real ) const {
  return static_cast<tsunami_lab::t_real>( 0 );
}

tsunami_lab::t_real
setup::TsunamiEvent1d::getBathymetry( tsunami_lab::t_real i_x,
                                       tsunami_lab::t_real      ) const {
  tsunami_lab::t_real l_bIn = getBathymetryRaw( i_x );
  tsunami_lab::t_real l_d   = getDisplacement( i_x );

  if( l_bIn < 0 ) {
    // b = min( b_in, -delta ) + d
    return std::min( l_bIn, -m_delta ) + l_d;
  } else {
    // b = max( b_in, delta ) + d
    return std::max( l_bIn,  m_delta ) + l_d;
  }
}
