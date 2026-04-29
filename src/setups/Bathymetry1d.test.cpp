/**
 * @section DESCRIPTION
 * Tests the one-dimensional dam break with bathymetry setup.
 **/
#include <catch2/catch.hpp>
#include "Bathymetry1d.h"

TEST_CASE( "Test the one-dimensional bathymetry setup.", "[Bathymetry1d]" ) {
  // dam at x=5, bathymetry step at x=7
  // left water surface 10, right water surface 8
  // left bathymetry -2, right bathymetry -4
  tsunami_lab::setups::Bathymetry1d l_bath( 10,   // heightLeft
                                             8,    // heightRight
                                             5,    // locationDam
                                            -2,    // bathymetryLeft
                                            -4,    // bathymetryRight
                                             7 );  // locationBathymetry

  // --- bathymetry ---
  // Left of bathymetry step (x < 7): bathymetryLeft = -2
  REQUIRE( l_bath.getBathymetry( 0, 0 ) == -2 );
  REQUIRE( l_bath.getBathymetry( 3, 0 ) == -2 );
  REQUIRE( l_bath.getBathymetry( 6, 0 ) == -2 );

  // Right of bathymetry step (x >= 7): bathymetryRight = -4
  REQUIRE( l_bath.getBathymetry( 7,  0 ) == -4 );
  REQUIRE( l_bath.getBathymetry( 10, 0 ) == -4 );
  REQUIRE( l_bath.getBathymetry( 20, 0 ) == -4 );

  // y-coordinate is ignored
  REQUIRE( l_bath.getBathymetry( 3, 99 ) == -2 );

  // --- height ---
  // Four possible combinations of (dam side) x (bathymetry side):

  // Quadrant 1: left of dam (x<5), left of bathy step (x<7)
  //   h = heightLeft - bathymetryLeft = 10 - (-2) = 12
  REQUIRE( l_bath.getHeight( 2, 0 ) == 12 );
  REQUIRE( l_bath.getHeight( 4, 0 ) == 12 );

  // Quadrant 2: left of dam (x<5), right of bathy step (x>=7)
  //   impossible when dam(5) < bathy(7), but the code handles it like this: h = heightLeft - bathymetryRight = 10 - (-4) = 14

  // Quadrant 3: right of dam (x>=5), left of bathy step (x<7)
  //   h = heightRight - bathymetryLeft = 8 - (-2) = 10
  REQUIRE( l_bath.getHeight( 5, 0 ) == 10 );
  REQUIRE( l_bath.getHeight( 6, 0 ) == 10 );

  // Quadrant 4: right of dam (x>=5), right of bathy step (x>=7)
  //   h = heightRight - bathymetryRight = 8 - (-4) = 12
  REQUIRE( l_bath.getHeight( 7,  0 ) == 12 );
  REQUIRE( l_bath.getHeight( 10, 0 ) == 12 );

  // y-coordinate is ignored
  REQUIRE( l_bath.getHeight( 2, 99 ) == 12 );

  // --- momentum (always zero) ---
  REQUIRE( l_bath.getMomentumX( 0,  0 ) == 0 );
  REQUIRE( l_bath.getMomentumX( 5,  0 ) == 0 );
  REQUIRE( l_bath.getMomentumX( 10, 5 ) == 0 );

  REQUIRE( l_bath.getMomentumY( 0,  0 ) == 0 );
  REQUIRE( l_bath.getMomentumY( 5,  0 ) == 0 );
  REQUIRE( l_bath.getMomentumY( 10, 5 ) == 0 );
}

TEST_CASE( "Test Bathymetry1d with dam right of bathymetry step.", "[Bathymetry1d]" ) {
  // Now dam at x=8, bathy step at x=4 — all four quadrants are reachable
  tsunami_lab::setups::Bathymetry1d l_bath( 10,   // heightLeft
                                             6,    // heightRight
                                             8,    // locationDam
                                             3,    // bathymetryLeft
                                             1,    // bathymetryRight
                                             4 );  // locationBathymetry

  // Quadrant 1: x<dam(8), x<bathy(4) means h = heightLeft - bathyLeft = 10 - 3 = 7
  REQUIRE( l_bath.getHeight( 2, 0 ) == 7 );

  // Quadrant 2: x<dam(8), x>=bathy(4) means h = heightLeft - bathyRight = 10 - 1 = 9
  REQUIRE( l_bath.getHeight( 5, 0 ) == 9 );
  REQUIRE( l_bath.getHeight( 7, 0 ) == 9 );

  // Quadrant 3: x>=dam(8), x<bathy(4) means it is impossible here (dam > bathy)
  // Quadrant 4: x>=dam(8), x>=bathy(4) means h = heightRight - bathyRight = 6 - 1 = 5
  REQUIRE( l_bath.getHeight(  8, 0 ) == 5 );
  REQUIRE( l_bath.getHeight( 12, 0 ) == 5 );

  // Bathymetry stays unchanged
  REQUIRE( l_bath.getBathymetry( 1, 0 ) == 3 );
  REQUIRE( l_bath.getBathymetry( 6, 0 ) == 1 );
}
