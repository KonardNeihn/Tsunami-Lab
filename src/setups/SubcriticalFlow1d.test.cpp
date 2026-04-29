/**
 * @section DESCRIPTION
 * Tests the one-dimensional subcritical flow over a bump setup.
 **/
#include <catch2/catch.hpp>
#include "SubcriticalFlow1d.h"
#include <cmath>

TEST_CASE( "Test the one-dimensional subcritical flow setup.", "[SubcriticalFlow1d]" ) {
  // bump centred at x=10, height 0.2, width 1.5, water surface 2.0, momentum 4.42
  tsunami_lab::setups::SubcriticalFlow1d l_flow( 10.0,
                                                  0.2,
                                                  1.5,
                                                  2.0,
                                                  4.42 );

  // --- bathymetry ---
  // b(x) = bumpHeight * exp( -(x - center)^2 / (2 * width^2) )

  // at the bump peak (x == center): b = bumpHeight = 0.2
  REQUIRE( l_flow.getBathymetry( 10.0, 0 ) == Approx( 0.2 ) );

  // far from the bump the bathymetry decays to essentially 0
  REQUIRE( l_flow.getBathymetry(  0.0, 0 ) == Approx( 0.0 ).margin( 1e-6 ) );
  REQUIRE( l_flow.getBathymetry( 25.0, 0 ) == Approx( 0.0 ).margin( 1e-6 ) );

  // at x=11.5 (one width to the right): b = 0.2 * exp(-0.5) ≈ 0.1213
  REQUIRE( l_flow.getBathymetry( 11.5, 0 ) == Approx( 0.2 * std::exp( -0.5 ) ) );

  // y-coordinate is ignored
  REQUIRE( l_flow.getBathymetry( 10.0, 99 ) == Approx( 0.2 ) );

  // --- height ---
  // For x far right of the bump (no pile): h = waterSurface - b(x)
  // At x=25: b≈0, h ≈ 2.0
  REQUIRE( l_flow.getHeight( 25.0, 0 ) == Approx( 2.0 ).margin( 1e-6 ) );

  // At the bump peak: h = 2.0 - 0.2 = 1.8
  REQUIRE( l_flow.getHeight( 10.0, 0 ) == Approx( 1.8 ) );

  // At x=11.5: h = 2.0 - 0.2*exp(-0.5)
  REQUIRE( l_flow.getHeight( 11.5, 0 ) == Approx( 2.0 - 0.2 * std::exp( -0.5 ) ) );

  // Left of the bump (x < center - 2*width = 7.0): small pile is added
  // h = max(0, (waterSurface + 0.1*exp(-(x-1)^2)) - b(x))
  // At x=1.0: pile = 0.1*exp(0) = 0.1, b≈0 -> h ≈ 2.1
  REQUIRE( l_flow.getHeight( 1.0, 0 ) == Approx( 2.1 ).margin( 1e-6 ) );

  // At x=4.0 (still left of 7.0): pile = 0.1*exp(-9) ≈ 0, h ≈ 2.0
  REQUIRE( l_flow.getHeight( 4.0, 0 ) == Approx( 2.0 ).margin( 1e-3 ) );

  // Height is never negative (wet/dry boundary)
  REQUIRE( l_flow.getHeight( 10.0, 0 ) >= 0.0 );

  // --- momentum ---

  // x-momentum is constant everywhere
  REQUIRE( l_flow.getMomentumX(  0.0, 0 ) == Approx( 4.42 ) );
  REQUIRE( l_flow.getMomentumX( 10.0, 0 ) == Approx( 4.42 ) );
  REQUIRE( l_flow.getMomentumX( 25.0, 5 ) == Approx( 4.42 ) );

  // y-momentum is always zero
  REQUIRE( l_flow.getMomentumY(  0.0, 0 ) == 0 );
  REQUIRE( l_flow.getMomentumY( 10.0, 0 ) == 0 );
  REQUIRE( l_flow.getMomentumY( 25.0, 5 ) == 0 );
}
