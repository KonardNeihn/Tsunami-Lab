/**
 * @section DESCRIPTION
 * Tests the artificial tsunami 2d setup.
 **/
#include <catch2/catch.hpp>
#include "ArtificialTsunami2d.h"

TEST_CASE( "ArtificialTsunami2d: Test m_bIn < 0 (Unterwater-Setup)", "[ArtificialTsunami2d]" ) {
    // Parameter: bIn = -10, roh = 5
    tsunami_lab::setups::ArtificialTsunami2d l_setup( -10.0, 5.0 );

    SECTION( "Momentumm is 0" ) {
        REQUIRE( l_setup.getMomentumX( 0, 0 ) == Approx( 0.0 ) );
        REQUIRE( l_setup.getMomentumY( 100, -50 ) == Approx( 0.0 ) );
    }

    SECTION( "Height calculation" ) {
        // m_bIn < 0 getHeight: max(-m_bIn, m_roh) = max(10, 5) = 10
        REQUIRE( l_setup.getHeight( 0, 0 ) == Approx( 10.0 ) );
        REQUIRE( l_setup.getHeight( 250, -100 ) == Approx( 10.0 ) ); 
    }

    SECTION( "Bathymetry at some points" ) {
        // min(m_bIn, -m_roh) = min(-10, -5) = -10
        
        // At (0,0) is l_d = 0 -> Bathymetry = -10 + 0 = -10
        REQUIRE( l_setup.getBathymetry( 0, 0 ) == Approx( -10.0 ) );

        // At (-250, 0) is l_d = 5 -> Bathymetry = -10 + 5 = -5
        REQUIRE( l_setup.getBathymetry( -250, 0 ) == Approx( -5.0 ) );

        // At (y = 500) is l_g = 0 -> Bathymetry = -10 + 0 = -10
        REQUIRE( l_setup.getBathymetry( 100, 500 ) == Approx( -10.0 ) );
    }
}

TEST_CASE( "ArtificialTsunami2d: Test with m_bIn >= 0 (Land-Setup)", "[ArtificialTsunami2d]" ) {
    // Parameter: bIn = 2, roh = 5
    tsunami_lab::setups::ArtificialTsunami2d l_setup( 2.0, 5.0 );

    SECTION( "Water Height m_bIn >= 0 has to be 0" ) {
        REQUIRE( l_setup.getHeight( 0, 0 ) == Approx( 0.0 ) );
    }

    SECTION( "Bathymetry for m_bIn >= 0" ) {
        // max(m_bIn, m_roh) = max(2, 5) = 5
        
        // At (0,0) is l_d = 0 -> Bathymetry = 5 + 0 = 5
        REQUIRE( l_setup.getBathymetry( 0, 0 ) == Approx( 5.0 ) );

        // At (-250, 0) is l_d = 5 -> Bathymetry = 5 + 5 = 10
        REQUIRE( l_setup.getBathymetry( -250, 0 ) == Approx( 10.0 ) );
    }
}

TEST_CASE( "ArtificialTsunami2d: if m_bIn > -m_roh", "[ArtificialTsunami2d]" ) {
    // Parameter: bIn = -2, roh = 5 (m_bIn ist negativ, aber kleiner als m_roh im Betrag)
    tsunami_lab::setups::ArtificialTsunami2d l_setup( -2.0, 5.0 );

    SECTION( "getHeight so positive: m_roh" ) {
        // max(-(-2), 5) = max(2, 5) = 5
        REQUIRE( l_setup.getHeight( 0, 0 ) == Approx( 5.0 ) );
    }

    SECTION( "getBathymetry so negative: -m_roh" ) {
        // min(-2, -5) = -5 -> At (0,0) so: -5 + 0 = -5
        REQUIRE( l_setup.getBathymetry( 0, 0 ) == Approx( -5.0 ) );
    }
}