/**
 * @author 
 *
 * @section DESCRIPTION
 * Tests the dam break setup.
 **/
#include <catch2/catch.hpp>
#include "RareRare1d.h"

TEST_CASE( "Test the one-dimensional rare-rare wave problem setup.", "[RareRare1d]" ) {
    // inputs are in this order: height, momentum, middle-location x-coordinate
  tsunami_lab::setups::RareRare1d l_rarerare( 25,
                                               5,
                                               3 );

  // "someone" designed these functions to work in 2 Dimensions so the second input is technically a y-coordinate that currently doesn't do anything
  // left side moves away from the middle - so it moves left (negative momentum)
  REQUIRE( l_rarerare.getHeight( 2, 0 ) == 25 );

  REQUIRE( l_rarerare.getMomentumX( 2, 0 ) == -5 );

  REQUIRE( l_rarerare.getMomentumY( 2, 0 ) == 0 );

  REQUIRE( l_rarerare.getHeight( 2, 5 ) == 25 );

  REQUIRE( l_rarerare.getMomentumX( 2, 5 ) == -5 );

  REQUIRE( l_rarerare.getMomentumY( 2, 2 ) == 0 );

  // right side moves away from the middle - so it moves right (positive momentum)
  REQUIRE( l_rarerare.getHeight( 4, 0 ) == 25 );

  REQUIRE( l_rarerare.getMomentumX( 4, 0 ) == 5 );

  REQUIRE( l_rarerare.getMomentumY( 4, 0 ) == 0 );

  REQUIRE( l_rarerare.getHeight( 4, 5 ) == 25 );

  REQUIRE( l_rarerare.getMomentumX( 4, 5 ) == 5 );

  REQUIRE( l_rarerare.getMomentumY( 4, 2 ) == 0 );  
}