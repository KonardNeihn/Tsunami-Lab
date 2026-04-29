/**
 * @section DESCRIPTION
 * Tests the one-dimensional hydraulic jump setup.
 **/
#include <catch2/catch.hpp>
#include "HydraulicJump1d.h"

TEST_CASE( "Test the one-dimensional hydraulic jump setup.", "[HydraulicJump1d]" ) {
  // obstacle top at -0.5, flat base at -1.0, momentum 4.42
  tsunami_lab::setups::HydraulicJump1d l_jump( -0.5,
                                               -1.0,
                                                4.42 );

  // --- bathymetry ---

  // outside obstacle region: flat base
  REQUIRE( l_jump.getBathymetry( 0,  0 ) == -1.0 );
  REQUIRE( l_jump.getBathymetry( 8,  0 ) == -1.0 );
  REQUIRE( l_jump.getBathymetry( 12, 0 ) == -1.0 );
  REQUIRE( l_jump.getBathymetry( 25, 0 ) == -1.0 );

  // inside of the obstacle (8 < x < 12):
  // b(x) = m_bObstacle - 0.05 * (x - 10)^2
  // at x=10 (peak): b = -0.5 - 0.0 = -0.5
  REQUIRE( l_jump.getBathymetry( 10, 0 ) == Approx( -0.5 ) );

  // at x=9: b = -0.5 - 0.05 * 1 = -0.55
  REQUIRE( l_jump.getBathymetry( 9, 0 ) == Approx( -0.55 ) );

  // at x=11: b = -0.5 - 0.05 * 1 = -0.55
  REQUIRE( l_jump.getBathymetry( 11, 0 ) == Approx( -0.55 ) );

  // --- height (= -bathymetry) ---
  // maybe i have used approximations too aggressively
  // outside obstacle region: h = -m_bBase = 1.0
  REQUIRE( l_jump.getHeight( 0,  0 ) == Approx( 1.0 ) );
  REQUIRE( l_jump.getHeight( 25, 0 ) == Approx( 1.0 ) );

  // at peak x=10: h = -(-0.5) = 0.5
  REQUIRE( l_jump.getHeight( 10, 0 ) == Approx( 0.5 ) );

  // at x=9: h = -(-0.55) = 0.55
  REQUIRE( l_jump.getHeight( 9, 0 ) == Approx( 0.55 ) );

  // at x=11: h = 0.55
  REQUIRE( l_jump.getHeight( 11, 0 ) == Approx( 0.55 ) );

  // y-coordinate is ignored
  REQUIRE( l_jump.getHeight( 0, 99 ) == Approx( 1.0 ) );

  // --- momentum ---

  // x-momentum is constant everywhere
  REQUIRE( l_jump.getMomentumX( 0,  0 ) == Approx( 4.42 ) );
  REQUIRE( l_jump.getMomentumX( 10, 0 ) == Approx( 4.42 ) );
  REQUIRE( l_jump.getMomentumX( 25, 5 ) == Approx( 4.42 ) );

  // y-momentum is always zero
  REQUIRE( l_jump.getMomentumY( 0,  0 ) == 0 );
  REQUIRE( l_jump.getMomentumY( 10, 0 ) == 0 );
  REQUIRE( l_jump.getMomentumY( 25, 5 ) == 0 );
}
