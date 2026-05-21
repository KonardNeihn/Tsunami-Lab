/**
 * @section DESCRIPTION
 * Unit tests for the two-dimensional wave propagation patch.
 **/
#include <catch2/catch.hpp>
#include "WavePropagation2d.h"

TEST_CASE( "Test the 2d wave propagation: ghost cell setup (outflow)", "[WavePropagation2d]" ) {
  // 3x3 interior cells for a 5x5 padded grid
  tsunami_lab::patches::WavePropagation2d l_waveProp( 3, 3 );

  // set a simple uniform height and momenta in interior cells
  for( tsunami_lab::t_idx l_cy = 0; l_cy < 3; l_cy++ ) {
    for( tsunami_lab::t_idx l_cx = 0; l_cx < 3; l_cx++ ) {
      l_waveProp.setHeight(    l_cx, l_cy, 5.0 );
      l_waveProp.setMomentumX( l_cx, l_cy, 1.0 );
      l_waveProp.setMomentumY( l_cx, l_cy, 2.0 );
      l_waveProp.setBathymetry(l_cx, l_cy, 0.0 );
    }
  }

  // outflow on all sides (all false)
  l_waveProp.setGhostCells( false, false, false, false );

  // ghost cells should mirror interior values unchanged
  tsunami_lab::t_real const * l_h  = l_waveProp.getHeight();
  tsunami_lab::t_real const * l_hu = l_waveProp.getMomentumX();
  tsunami_lab::t_real const * l_hv = l_waveProp.getMomentumY();
  tsunami_lab::t_idx           l_s  = l_waveProp.getStride();  // = nX + 2 = 5

  // all interior cells should be unchanged
  for( tsunami_lab::t_idx l_cy = 0; l_cy < 3; l_cy++ ) {
    for( tsunami_lab::t_idx l_cx = 0; l_cx < 3; l_cx++ ) {
      REQUIRE( l_h [ l_cy * l_s + l_cx ] == Approx( 5.0 ) );
      REQUIRE( l_hu[ l_cy * l_s + l_cx ] == Approx( 1.0 ) );
      REQUIRE( l_hv[ l_cy * l_s + l_cx ] == Approx( 2.0 ) );
    }
  }
}

TEST_CASE( "Test the 2d wave propagation: ghost cell setup (reflecting)", "[WavePropagation2d]" ) {
  tsunami_lab::patches::WavePropagation2d l_waveProp( 3, 3 );

  for( tsunami_lab::t_idx l_cy = 0; l_cy < 3; l_cy++ ) {
    for( tsunami_lab::t_idx l_cx = 0; l_cx < 3; l_cx++ ) {
      l_waveProp.setHeight(    l_cx, l_cy, 4.0 );
      l_waveProp.setMomentumX( l_cx, l_cy, 3.0 );
      l_waveProp.setMomentumY( l_cx, l_cy, 7.0 );
      l_waveProp.setBathymetry(l_cx, l_cy, 0.0 );
    }
  }

  // reflecting on all four sides
  l_waveProp.setGhostCells( true, true, true, true );

  tsunami_lab::t_real const * l_h  = l_waveProp.getHeight();
  tsunami_lab::t_real const * l_hu = l_waveProp.getMomentumX();
  tsunami_lab::t_real const * l_hv = l_waveProp.getMomentumY();
  tsunami_lab::t_idx           l_s  = l_waveProp.getStride();  // 5

  // height is always copied (never negated) regardless of reflection
  for( tsunami_lab::t_idx l_cy = 0; l_cy < 3; l_cy++ ) {
    for( tsunami_lab::t_idx l_cx = 0; l_cx < 3; l_cx++ ) {
      REQUIRE( l_h[ l_cy * l_s + l_cx ] == Approx( 4.0 ) );
    }
  }

  // left ghost column: hu negated, hv unchanged
  for( tsunami_lab::t_idx l_cy = 0; l_cy < 3; l_cy++ ) {
    REQUIRE( l_hu[ l_cy * l_s - 1 ] == Approx( -3.0 ) );  // ghost ix=-1 relative to interior
    REQUIRE( l_hv[ l_cy * l_s - 1 ] == Approx(  7.0 ) );
  }

  // right ghost column: hu negated, hv unchanged
  for( tsunami_lab::t_idx l_cy = 0; l_cy < 3; l_cy++ ) {
    REQUIRE( l_hu[ l_cy * l_s + 3 ] == Approx( -3.0 ) );
    REQUIRE( l_hv[ l_cy * l_s + 3 ] == Approx(  7.0 ) );
  }

  // bottom ghost row: hv negated, hu unchanged
  for( tsunami_lab::t_idx l_cx = 0; l_cx < 3; l_cx++ ) {
    REQUIRE( l_hu[ l_cx - l_s ] == Approx(  3.0 ) );
    REQUIRE( l_hv[ l_cx - l_s ] == Approx( -7.0 ) );
  }

  // top ghost row: hv negated, hu unchanged
  for( tsunami_lab::t_idx l_cx = 0; l_cx < 3; l_cx++ ) {
    REQUIRE( l_hu[ 3 * l_s + l_cx ] == Approx(  3.0 ) );
    REQUIRE( l_hv[ 3 * l_s + l_cx ] == Approx( -7.0 ) );
  }
}

TEST_CASE( "Test the 2d wave propagation: symmetry of circular dam break", "[WavePropagation2d]" ) {
  // Use a small square grid and a circular initial condition centred at the origin.
  // After one time step the solution must remain symmetric across both axes.
  tsunami_lab::t_idx l_n   = 50;
  tsunami_lab::t_real l_dxy = 2.0;   // cell size [m]

  tsunami_lab::patches::WavePropagation2d l_waveProp( l_n, l_n );

  // set circular dam break: h=10 inside radius 10, h=5 outside
  for( tsunami_lab::t_idx l_cy = 0; l_cy < l_n; l_cy++ ) {
    for( tsunami_lab::t_idx l_cx = 0; l_cx < l_n; l_cx++ ) {
      // cell centre in [-50, 50]^2
      tsunami_lab::t_real l_x = (l_cx + 0.5) * l_dxy - 50.0;
      tsunami_lab::t_real l_y = (l_cy + 0.5) * l_dxy - 50.0;
      tsunami_lab::t_real l_r = std::sqrt( l_x * l_x + l_y * l_y );

      l_waveProp.setHeight(    l_cx, l_cy, l_r < 10.0 ? 10.0 : 5.0 );
      l_waveProp.setMomentumX( l_cx, l_cy, 0.0 );
      l_waveProp.setMomentumY( l_cx, l_cy, 0.0 );
      l_waveProp.setBathymetry(l_cx, l_cy, 0.0 );
    }
  }

  l_waveProp.setGhostCells( false, false, false, false );

  // dt from CFL: max wave speed = sqrt(9.81 * 10) ~ 9.9 m/s, CFL 0.45
  tsunami_lab::t_real l_dt      = 0.45 * l_dxy / std::sqrt( 9.81 * 10.0 );
  tsunami_lab::t_real l_scaling = l_dt / l_dxy;

  l_waveProp.timeStep( l_scaling );

  tsunami_lab::t_real const * l_h = l_waveProp.getHeight();
  tsunami_lab::t_idx           l_s = l_waveProp.getStride();

  // symmetry check: h(cx, cy) == h(n-1-cx, cy)  (left-right)
  //                 h(cx, cy) == h(cx, n-1-cy)  (top-bottom)
  for( tsunami_lab::t_idx l_cy = 0; l_cy < l_n; l_cy++ ) {
    for( tsunami_lab::t_idx l_cx = 0; l_cx < l_n / 2; l_cx++ ) {
      tsunami_lab::t_real l_hL = l_h[ l_cy * l_s + l_cx ];
      tsunami_lab::t_real l_hR = l_h[ l_cy * l_s + (l_n - 1 - l_cx) ];
      REQUIRE( l_hL == Approx( l_hR ).margin( 1e-10 ) );

      tsunami_lab::t_real l_hB = l_h[ l_cx       * l_s + l_cy ];
      tsunami_lab::t_real l_hT = l_h[ (l_n-1-l_cx) * l_s + l_cy ];
      REQUIRE( l_hB == Approx( l_hT ).margin( 1e-10 ) );
    }
  }
}

TEST_CASE( "Test the 2d wave propagation: steady state (uniform height, zero momentum)", "[WavePropagation2d]" ) {
  // A uniform lake at rest should remain unchanged after any number of time steps.
  tsunami_lab::patches::WavePropagation2d l_waveProp( 10, 10 );

  for( tsunami_lab::t_idx l_cy = 0; l_cy < 10; l_cy++ ) {
    for( tsunami_lab::t_idx l_cx = 0; l_cx < 10; l_cx++ ) {
      l_waveProp.setHeight(    l_cx, l_cy, 3.0 );
      l_waveProp.setMomentumX( l_cx, l_cy, 0.0 );
      l_waveProp.setMomentumY( l_cx, l_cy, 0.0 );
      l_waveProp.setBathymetry(l_cx, l_cy, 0.0 );
    }
  }

  l_waveProp.setGhostCells( false, false, false, false );

  tsunami_lab::t_real l_dt      = 0.45 * 1.0 / std::sqrt( 9.81 * 3.0 );
  tsunami_lab::t_real l_scaling = l_dt / 1.0;

  // run several steps
  for( int l_step = 0; l_step < 10; l_step++ ) {
    l_waveProp.setGhostCells( false, false, false, false );
    l_waveProp.timeStep( l_scaling );
  }

  tsunami_lab::t_real const * l_h  = l_waveProp.getHeight();
  tsunami_lab::t_real const * l_hu = l_waveProp.getMomentumX();
  tsunami_lab::t_real const * l_hv = l_waveProp.getMomentumY();
  tsunami_lab::t_idx           l_s  = l_waveProp.getStride();

  for( tsunami_lab::t_idx l_cy = 0; l_cy < 10; l_cy++ ) {
    for( tsunami_lab::t_idx l_cx = 0; l_cx < 10; l_cx++ ) {
      REQUIRE( l_h [ l_cy * l_s + l_cx ] == Approx( 3.0 ).margin( 1e-10 ) );
      REQUIRE( l_hu[ l_cy * l_s + l_cx ] == Approx( 0.0 ).margin( 1e-10 ) );
      REQUIRE( l_hv[ l_cy * l_s + l_cx ] == Approx( 0.0 ).margin( 1e-10 ) );
    }
  }
}