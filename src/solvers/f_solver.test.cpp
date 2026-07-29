/**
 * @author NOCH NICHT ERLEDIGT 
 *
 * @section DESCRIPTION
 * Unit tests of the f_wave solver.
 **/
#include <catch2/catch.hpp>
#include "f_solver.h"

// Clang-compliant testing
struct FWaveTestAccessor {
  static void waveSpeeds(tsunami_lab::t_real i_hL, tsunami_lab::t_real i_hR, 
                         tsunami_lab::t_real i_uL, tsunami_lab::t_real i_uR, 
                         tsunami_lab::t_real &o_waveSpeedL, tsunami_lab::t_real &o_waveSpeedR) {
    tsunami_lab::solvers::f_wave::waveSpeeds(i_hL, i_hR, i_uL, i_uR, o_waveSpeedL, o_waveSpeedR);
  }

  static void waveFlux(tsunami_lab::t_real i_hL, tsunami_lab::t_real i_hR, 
                       tsunami_lab::t_real i_huL, tsunami_lab::t_real i_huR, 
                       tsunami_lab::t_real i_bL, tsunami_lab::t_real i_bR, 
                       tsunami_lab::t_real &o_fluxL, tsunami_lab::t_real &o_fluxR) {
    tsunami_lab::solvers::f_wave::waveFlux(i_hL, i_hR, i_huL, i_huR, i_bL, i_bR, o_fluxL, o_fluxR);
  }
};

TEST_CASE( "Test the derivation of the f-wave speeds.", "[FWaveSpeeds]" ) {

  float l_waveSpeedL = 0;
  float l_waveSpeedR = 0;
  FWaveTestAccessor::waveSpeeds( 10,
                                         9,
                                         -3,
                                         3,
                                         l_waveSpeedL,
                                         l_waveSpeedR );

  REQUIRE( l_waveSpeedL == Approx( -12.90285 ) );
  REQUIRE( l_waveSpeedR == Approx(  12.39467 ) );
}

TEST_CASE( "Test the F-WaveFlux.", "[FWaveFlux]" ) {
  /*
   * Test case:
   *  h:   10 |  8
   *  u:   0  |  0
   *  hu:  0  |  0
   *
   *  links:  0
   *          0^2 + 1/2 * g * 10^2  = 490.3325
   *  rechts: 0
   *          0^2 + 1/2 * g * 8^2   = 313.8128
   *
   *  differenz:  0
   *              313.8128 - 490.3325 = -176.5197
   *              
   */
  float l_fluxdiffH = 0;
  float l_fluxdiffHu = 0;

  FWaveTestAccessor::waveFlux(10,
                                        8,
                                        0,
                                        0,
                                        0, // needs to be changed
                                        0, // needs to be changed
                                        l_fluxdiffH,
                                        l_fluxdiffHu);

  REQUIRE(l_fluxdiffH == Approx(0));
  REQUIRE(l_fluxdiffHu == Approx(-176.5197));
}

TEST_CASE( "Test the derivation of the f-wave net-updates.", "[FWaveUpdates]" ) {
  /*
   * Test case (supersonic):
   *
   *      left | right
   *  h:     10 | 8
   *  u:    -10 | -10
   *  hu:   -100 | -80
   *
   *  h (10 + 8) / 2 = 9 
   *  u (sqrt(10) * -10 + sqrt(8) * -10) / (sqrt(10) + sqrt(8)) =  -10
   *
   *  e1  -10 - sqrt(9.80665 * 9) = -19.39467
   *  e2  -10 + sqrt(9.80665 * 9) = -0.6053286379990919
   *
   *  flux
   *  10 * 100 + 1/2 * 9.80665 * 10^2 = 1490.3325
   *  8 * 100 + 1/2 * 9.80665 * 8^2  = 1113.8128
   *  -80 - -100 = 20
   *  1113.8128 - 1490.3325 = -376.5197
   *
   *  [a1, a2] = [[1, -19.39467] [1, -0.6053286379990919]]^-1 * [20, -376.5197] =  [19.3947 0.605327]
   *  19.3947 + 0.605327 = 20
   */
  float l_netUpdatesL[2] = {0, 0};
  float l_netUpdatesR[2] = {0, 0};

  tsunami_lab::solvers::f_wave::netUpdates( 10,
                                         8,
                                         -100,
                                         -80,
                                         0,
                                         0,
                                         l_netUpdatesL,
                                         l_netUpdatesR );

  REQUIRE(l_netUpdatesL[0] == Approx(20));
  REQUIRE(l_netUpdatesL[1] == Approx(-376.5197));

  REQUIRE(l_netUpdatesR[0] == Approx(0));
  REQUIRE(l_netUpdatesR[1] == Approx(0));

  /*
   * Test case (trivial steady state):
   *
   *     left | right
   *   h:  10 | 10
   *  hu:   0 |  0
   */
  tsunami_lab::solvers::f_wave::netUpdates( 10,
                                         10,
                                         0,
                                         0,
                                         0,
                                         0,
                                         l_netUpdatesL,
                                         l_netUpdatesR );

  REQUIRE( l_netUpdatesL[0] == Approx(0) );
  REQUIRE( l_netUpdatesL[1] == Approx(0) );

  REQUIRE( l_netUpdatesR[0] == Approx(0) );
  REQUIRE( l_netUpdatesR[1] == Approx(0) );
}