/**
 * @author 
 *
 * @section DESCRIPTION
 * Tests the dam break setup.
 **/
#include <catch2/catch.hpp>
#include "ShockShock1d.h"

TEST_CASE( "Test the one-dimensional shock-shock wave problem setup.", "[ShockShock1d]" ) {
    // inputs are in this order: height, momentum, middle-location x-coordinate
  tsunami_lab::setups::ShockShock1d l_shockshock( 25,
                                                   5,
                                                   3 );

  // "someone" designed these functions two work in 2 Dimensions so the second input is technically a y-coordinate that currently doesn't do anything
  // left side moves toward the middle - so it moves right (positive momentum)
  REQUIRE( l_shockshock.getHeight( 2, 0 ) == 25 );

  REQUIRE( l_shockshock.getMomentumX( 2, 0 ) == 5 );

  REQUIRE( l_shockshock.getMomentumY( 2, 0 ) == 0 );

  REQUIRE( l_shockshock.getHeight( 2, 5 ) == 25 );

  REQUIRE( l_shockshock.getMomentumX( 2, 5 ) == 5 );

  REQUIRE( l_shockshock.getMomentumY( 2, 2 ) == 0 );

  // right side moves toward the middle - so it moves left (negative momentum)
  REQUIRE( l_shockshock.getHeight( 4, 0 ) == 25 );

  REQUIRE( l_shockshock.getMomentumX( 4, 0 ) == -5 );

  REQUIRE( l_shockshock.getMomentumY( 4, 0 ) == 0 );

  REQUIRE( l_shockshock.getHeight( 4, 5 ) == 25 );

  REQUIRE( l_shockshock.getMomentumX( 4, 5 ) == -5 );

  REQUIRE( l_shockshock.getMomentumY( 4, 2 ) == 0 );  
}