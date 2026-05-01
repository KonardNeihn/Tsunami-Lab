/**
 *
 * @section DESCRIPTION
 * Tests the TsunamiEvent1d setup.
 **/
#include <catch2/catch.hpp>
#include "TsunamiEvent1d.h"

// Helpe to construct a setup using the bundled test bathymetry CSV.
// Adjust the path to wherever your test data lives.
static const std::string BATHY_CSV = "src/bathymetry/output/03_dem_03.csv";

TEST_CASE( "Test the one-dimensional tsunami event setup.", "[TsunamiEvent1d]" ) {
  tsunami_lab::setups::TsunamiEvent1d l_tsunami( BATHY_CSV );
 
  // track_location runs 0 to 440 km 
  // height > 0 = land (near x = 0),  height < 0 = ocean 
  // Displacement zone: 175 000 m < x < 250 000 m 
 
  // -------------------------------------------------------------------------
  SECTION( "Momentum is always zero (water starts at rest)" ) {
    REQUIRE( l_tsunami.getMomentumX(      0, 0 ) == 0 );
    REQUIRE( l_tsunami.getMomentumX( 100000, 0 ) == 0 );
    REQUIRE( l_tsunami.getMomentumX( 210000, 0 ) == 0 );  // inside displacement zone
    REQUIRE( l_tsunami.getMomentumX( 440000, 0 ) == 0 );
 
    REQUIRE( l_tsunami.getMomentumY(      0, 0 ) == 0 );
    REQUIRE( l_tsunami.getMomentumY( 210000, 0 ) == 0 );
    REQUIRE( l_tsunami.getMomentumY( 440000, 5 ) == 0 );
  }
 
  // -------------------------------------------------------------------------
  SECTION( "Height is zero on dry land (positive bathymetry near x = 0)" ) {
    // The very first CSV row has height = +14.7 m, so x ~ 0 is land.
    REQUIRE( l_tsunami.getHeight(   0, 0 ) == 0 );
    REQUIRE( l_tsunami.getHeight( 100, 0 ) == 0 );  // still within the coastal land strip
  }
 
  // -------------------------------------------------------------------------
  SECTION( "Height is positive in open ocean (negative bathymetry, outside displacement zone)" ) {
    // x = 50 000 m is well into the ocean and far from the 175-250 km displacement zone.
    REQUIRE( l_tsunami.getHeight( 50000, 0 ) > 0 );
    REQUIRE( l_tsunami.getHeight( 50000, 7 ) > 0 );  // y-coordinate must be irrelevant
  }
 
  // -------------------------------------------------------------------------
  SECTION( "Displacement zone shifts height relative to outside (175 km < x < 250 km)" ) {
    // x = 50 000 m : undisturbed reference point in open ocean
    // x = 210 000 m: inside the sine displacement pulse (amplitude 1000 m)
    // Both are underwater so height > 0 in both cases.
    tsunami_lab::t_real l_hRef = l_tsunami.getHeight(  50000, 0 );
    tsunami_lab::t_real l_hDis = l_tsunami.getHeight( 210000, 0 );
 
    REQUIRE( l_hRef > 0 );
    REQUIRE( l_hDis > 0 );
 
    // The 1000 m displacement pulse is large enough that the two heights must be different
    REQUIRE( l_hDis != l_hRef );
  }
 
  // -------------------------------------------------------------------------
  SECTION( "Bathymetry sign matches CSV: ocean is negative, land is positive" ) {
    // x = 0: first CSV row has height = +14.7 m  which means  land which means  b > 0
    REQUIRE( l_tsunami.getBathymetry( 0, 0 ) > 0 );
 
    // x = 50 000 m: well into the ocean, so  b < 0
    REQUIRE( l_tsunami.getBathymetry( 50000, 0 ) < 0 );
 
    // x = 440 000 m: deep ocean at the far end of the CSV means b < 0
    REQUIRE( l_tsunami.getBathymetry( 440000, 0 ) < 0 );
  }
 
  // -------------------------------------------------------------------------
  SECTION( "Bathymetry clamping: |b| >= delta everywhere" ) {
    // getBathymetry clamps so that ocean cells return <= -delta
    // and land cells return >= +delta (delta = 20 m by default in the header).
    const tsunami_lab::t_real l_delta = 20;
 
    auto check = []( tsunami_lab::t_real l_b, tsunami_lab::t_real l_d ) {
      return l_b <= -l_d || l_b >= l_d;
    };
 
    REQUIRE( check( l_tsunami.getBathymetry(      0, 0 ), l_delta ) );  // land
    REQUIRE( check( l_tsunami.getBathymetry(  50000, 0 ), l_delta ) );  // ocean
    REQUIRE( check( l_tsunami.getBathymetry( 210000, 0 ), l_delta ) );  // displacement zone
    REQUIRE( check( l_tsunami.getBathymetry( 440000, 0 ), l_delta ) );  // deep ocean
  }
 
  // -------------------------------------------------------------------------
  SECTION( "y-coordinate has no effect on any quantity" ) {
    REQUIRE( l_tsunami.getHeight(     50000, 0 ) == l_tsunami.getHeight(     50000, 99 ) );
    REQUIRE( l_tsunami.getMomentumX(  50000, 0 ) == l_tsunami.getMomentumX(  50000, 99 ) );
    REQUIRE( l_tsunami.getMomentumY(  50000, 0 ) == l_tsunami.getMomentumY(  50000, 99 ) );
    REQUIRE( l_tsunami.getBathymetry( 50000, 0 ) == l_tsunami.getBathymetry( 50000, 99 ) );
  }
}