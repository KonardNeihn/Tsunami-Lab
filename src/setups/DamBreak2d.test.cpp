/**
 * @section DESCRIPTION
 * Tests the DamBreak2d setup.
 **/
#include <catch2/catch.hpp>
#include "DamBreak2d.h"

TEST_CASE( "DamBreak2d: CircularDamBreak at (0,0)", "[DamBreak2d]" ) {
    // Parameter: heightLeft = 10.0, heightRight = 2.0, DamX = 0.0, DamY = 0.0, Radius = 5.0
    tsunami_lab::setups::DamBreak2d l_damBreak( 10.0, 2.0, 0.0, 0.0, 5.0 );

    SECTION( "Momentum is 0 everywhere" ) {
        REQUIRE( l_damBreak.getMomentumX( 0.0, 0.0 ) == Approx( 0.0 ) );
        REQUIRE( l_damBreak.getMomentumY( 3.0, 4.0 ) == Approx( 0.0 ) );
    }

    SECTION( "Inside of the dam with distance < radius" ) {
        // Middle (0,0) -> d = 0 < 5
        REQUIRE( l_damBreak.getHeight( 0.0, 0.0 ) == Approx( 10.0 ) );

        // inside the dam (z.B. x = 3, y = 0 -> d = 3 < 5)
        REQUIRE( l_damBreak.getHeight( 3.0, 0.0 )  == Approx( 10.0 ) );
        REQUIRE( l_damBreak.getHeight( -3.0, 0.0 ) == Approx( 10.0 ) );
        REQUIRE( l_damBreak.getHeight( 0.0, 4.0 )  == Approx( 10.0 ) );
        REQUIRE( l_damBreak.getHeight( 0.0, -4.0 ) == Approx( 10.0 ) );

        // more diagonally (x = 3, y = 3 -> d^2 = 18 < 25)
        REQUIRE( l_damBreak.getHeight( 3.0, 3.0 ) == Approx( 10.0 ) );
    }

    SECTION( "outside the dam distance > radius" ) {
        // outside (z.B. x = 6, y = 0 -> d = 6 > 5)
        REQUIRE( l_damBreak.getHeight( 6.0, 0.0 )  == Approx( 2.0 ) );
        REQUIRE( l_damBreak.getHeight( -6.0, 0.0 ) == Approx( 2.0 ) );
        REQUIRE( l_damBreak.getHeight( 0.0, 6.0 )  == Approx( 2.0 ) );

        // more diagonally (x = 4, y = 4 -> d^2 = 32 > 25)
        REQUIRE( l_damBreak.getHeight( 4.0, 4.0 ) == Approx( 2.0 ) );
    }

    SECTION( "exactly on the border (distance == radius)" ) {
        // at x = 5, y = 0 ist d^2 = 25 == r^2 (25) -> "else" (heightRight)
        REQUIRE( l_damBreak.getHeight( 5.0, 0.0 ) == Approx( 2.0 ) );

        // 3, 4, 5: 3^2 + 4^2 = 25 == 5^2
        REQUIRE( l_damBreak.getHeight( 3.0, 4.0 ) == Approx( 2.0 ) );
    }
}

TEST_CASE( "DamBreak2d: middle is in negative area", "[DamBreak2d]" ) {
    // Dam at (-10, -20), Radius 10
    tsunami_lab::setups::DamBreak2d l_damBreak( 15.0, 1.0, -10.0, -20.0, 10.0 );

    SECTION( "test middle point" ) {
        REQUIRE( l_damBreak.getHeight( -10.0, -20.0 ) == Approx( 15.0 ) );
    }

    SECTION( "test distance" ) {
        // (-10 + 5, -20) = (-5, -20) -> Distancw = 5 < 10 (inside)
        REQUIRE( l_damBreak.getHeight( -5.0, -20.0 ) == Approx( 15.0 ) );

        // (-10 + 12, -20) = (2, -20) -> Distancw = 12 > 10 (outside)
        REQUIRE( l_damBreak.getHeight( 2.0, -20.0 ) == Approx( 1.0 ) );
    }
}