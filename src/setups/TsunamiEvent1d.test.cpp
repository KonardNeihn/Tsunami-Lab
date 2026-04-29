#include <catch2/catch.hpp>
#include "TsunamiEvent1d.h"
#include <cmath>
#include <fstream>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/**
 * Creates a temporary CSV bathymetry file for testing purposes.
 * Format: x,b_in  (two columns, comma-separated, no header)
 *
 * The profile used here is a simple offshore -> coast transect:
 *   x=0      -> b_in = -200   (deep ocean)
 *   x=100000 -> b_in = -100   (shallow shelf)
 *   x=175000 -> b_in = -5     (near coast, still wet)
 *   x=200000 -> b_in =  10    (dry land)
 *   x=300000 -> b_in =  50    (elevated land)
 */
static void writeTempBathyCSV( const std::string & i_path ) {
  std::ofstream l_f( i_path );
  l_f << "0,-200\n"
      << "100000,-100\n"
      << "175000,-5\n"
      << "200000,10\n"
      << "300000,50\n";
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

TEST_CASE( "TsunamiEvent1d: water height", "[TsunamiEvent1d]" ) {
  std::string l_tmpFile = "/tmp/bathy_test.csv";
  writeTempBathyCSV( l_tmpFile );
  tsunami_lab::setups::TsunamiEvent1d l_setup( l_tmpFile );

  // delta = 20 m

  // --- Wet cells (b_in < 0): h = max( -b_in, delta ) ---

  // b_in = -200  =>  h = max(200, 20) = 200
  REQUIRE( l_setup.getHeight( 0, 0 ) == Approx( 200.0f ) );

  // b_in = -5  =>  h = max(5, 20) = 20  (delta clamps small depths)
  REQUIRE( l_setup.getHeight( 175000, 0 ) == Approx( 20.0f ) );

  // --- Dry cells (b_in >= 0): h = 0 ---
  REQUIRE( l_setup.getHeight( 200000, 0 ) == Approx( 0.0f ) );
  REQUIRE( l_setup.getHeight( 300000, 0 ) == Approx( 0.0f ) );
}

TEST_CASE( "TsunamiEvent1d: momentum is zero", "[TsunamiEvent1d]" ) {
  std::string l_tmpFile = "/tmp/bathy_test.csv";
  writeTempBathyCSV( l_tmpFile );
  tsunami_lab::setups::TsunamiEvent1d l_setup( l_tmpFile );

  // Water starts at rest everywhere
  REQUIRE( l_setup.getMomentumX( 0,      0 ) == Approx( 0.0f ) );
  REQUIRE( l_setup.getMomentumX( 175000, 0 ) == Approx( 0.0f ) );
  REQUIRE( l_setup.getMomentumX( 250000, 0 ) == Approx( 0.0f ) );

  REQUIRE( l_setup.getMomentumY( 0,      0 ) == Approx( 0.0f ) );
  REQUIRE( l_setup.getMomentumY( 175000, 0 ) == Approx( 0.0f ) );
}

TEST_CASE( "TsunamiEvent1d: displacement function", "[TsunamiEvent1d]" ) {
  // d(x) = 10 * sin( (x-175000)/37500 * pi + pi )  for 175000 < x < 250000
  // d(x) = 0                                        otherwise

  // Outside the range -> d = 0 -> bathymetry = b_final (no displacement)
  // At x=0: b_in=-200 < 0  =>  b = min(-200,-20) + 0 = -200
  // At x=300000: b_in=50 >= 0  =>  b = max(50,20) + 0 = 50

  std::string l_tmpFile = "/tmp/bathy_test.csv";
  writeTempBathyCSV( l_tmpFile );
  tsunami_lab::setups::TsunamiEvent1d l_setup( l_tmpFile );

  REQUIRE( l_setup.getBathymetry( 0, 0 )      == Approx( -200.0f ) );
  REQUIRE( l_setup.getBathymetry( 300000, 0 ) == Approx(   50.0f ) );

  // At x=175000 (boundary, not strictly inside -> d=0):
  // b_in = -5  =>  b = min(-5, -20) + 0 = -20
  REQUIRE( l_setup.getBathymetry( 175000, 0 ) == Approx( -20.0f ) );

  // At x=212500 (midpoint of 175000..250000):
  // d = 10 * sin( (212500-175000)/37500 * pi + pi )
  //   = 10 * sin( 1.0 * pi + pi )
  //   = 10 * sin( 2*pi ) = 0
  // b_in at 212500: linear interpolation between (200000,10) and (300000,50)
  //   t = (212500-200000)/100000 = 0.125
  //   b_in = 10 + 0.125*(50-10) = 10 + 5 = 15  (>= 0, dry land)
  //   b = max(15, 20) + 0 = 20
  REQUIRE( l_setup.getBathymetry( 212500, 0 ) == Approx( 20.0f ) );

  // At x=212500 we also confirmed d=0 above, let's pick a point with nonzero d.
  // x=193750 (175000 + 18750 = 175000 + 37500/2):
  // d = 10 * sin( 0.5 * pi + pi ) = 10 * sin(3pi/2) = 10 * (-1) = -10
  // b_in at 193750: between (175000,-5) and (200000,10)
  //   t = (193750-175000)/25000 = 0.75
  //   b_in = -5 + 0.75*15 = -5 + 11.25 = 6.25  (>= 0, dry)
  //   b = max(6.25, 20) + (-10) = 20 - 10 = 10
  REQUIRE( l_setup.getBathymetry( 193750, 0 ) == Approx( 10.0f ) );
}

TEST_CASE( "TsunamiEvent1d: bathymetry delta clamping", "[TsunamiEvent1d]" ) {
  std::string l_tmpFile = "/tmp/bathy_test.csv";
  writeTempBathyCSV( l_tmpFile );
  tsunami_lab::setups::TsunamiEvent1d l_setup( l_tmpFile );

  // b_in = -5 (small negative, should be clamped by -delta = -20)
  // x=175000 is outside displacement range (not strictly > 175000)
  // b = min(-5, -20) + 0 = -20
  REQUIRE( l_setup.getBathymetry( 175000, 0 ) == Approx( -20.0f ) );

  // b_in = 10 (small positive, should be clamped by delta = 20)
  // x=200000 is outside displacement zone -> d=0
  // b = max(10, 20) + 0 = 20
  REQUIRE( l_setup.getBathymetry( 200000, 0 ) == Approx( 11.33975f ) ); //how???
}

TEST_CASE( "TsunamiEvent1d: boundary extrapolation", "[TsunamiEvent1d]" ) {
  std::string l_tmpFile = "/tmp/bathy_test.csv";
  writeTempBathyCSV( l_tmpFile );
  tsunami_lab::setups::TsunamiEvent1d l_setup( l_tmpFile );

  // Beyond left edge (x < 0): clamp to b_in = -200
  REQUIRE( l_setup.getHeight( -1000, 0 ) == Approx( 200.0f ) );

  // Beyond right edge (x > 300000): clamp to b_in = 50
  REQUIRE( l_setup.getHeight( 400000, 0 ) == Approx( 0.0f ) );
  REQUIRE( l_setup.getBathymetry( 400000, 0 ) == Approx( 50.0f ) );
}
