/**
 * @author NOCH NICHT ERLEDIGT 
 *
 * @section DESCRIPTION
 * Unit tests of the f_wave solver.
 **/
#include <catch2/catch.hpp>
#define private public
#include "f_solver.h"
#undef public

TEST_CASE( "Test the derivation of the f-wave speeds.", "[FWaveSpeeds]" ) {
   /*
    * (gleich wie bei roe)
    *
    * Test case:
    *  h: 10 | 9
    *  u: -3 | 3
    *
    * f-wave height: 9.5
    * f-wave velocity: (sqrt(10) * -3 + 3 * 3) / ( sqrt(10) + sqrt(9) )
    *                   = -0.0790021169691720
    * f-wave speeds:  s1 = -0.079002116969172024 - sqrt(9.80665 * 9.5) = -9.7311093998375095
    *                 s2 = -0.079002116969172024 + sqrt(9.80665 * 9.5) =  9.5731051658991654
    */
  float l_waveSpeedL = 0;
  float l_waveSpeedR = 0;
  tsunami_lab::solvers::f_wave::waveSpeeds( 10,
                                         9,
                                         -3,
                                         3,
                                         l_waveSpeedL,
                                         l_waveSpeedR );

  REQUIRE( l_waveSpeedL == Approx( -9.7311093998375095 ) );
  REQUIRE( l_waveSpeedR == Approx(  9.5731051658991654 ) );
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

  tsunami_lab::solvers::f_wave::waveFlux(10,
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
                                         l_netUpdatesL,
                                         l_netUpdatesR );

  REQUIRE( l_netUpdatesL[0] == Approx(0) );
  REQUIRE( l_netUpdatesL[1] == Approx(0) );

  REQUIRE( l_netUpdatesR[0] == Approx(0) );
  REQUIRE( l_netUpdatesR[1] == Approx(0) );
}