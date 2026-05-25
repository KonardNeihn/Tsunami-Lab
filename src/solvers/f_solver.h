/**
 * @author LL
 *
 * @section DESCRIPTION
 * F-Wave solver for the one-dimensional shallow water equations.
 **/
#ifndef TSUNAMI_LAB_SOLVERS_F_WAVE
#define TSUNAMI_LAB_SOLVERS_F_WAVE

#include "../constants.h"

namespace tsunami_lab {
  namespace solvers {
    class f_wave;
  }
}

class tsunami_lab::solvers::f_wave {
  private:
    //! square root of gravity
    static t_real constexpr m_gSqrt = 3.131557121;
    //! gravity as is written in 1.Riemann Solver
    static t_real constexpr m_g = 9.80665;
    //! temporary static threshold for dry-state purposes
    static constexpr t_real m_dryThreshold = 1e-4;

    /**
     * Computes the wave speeds.
     *
     * @param i_hL height of the left side.
     * @param i_hR height of the right side.
     * @param i_uL particle velocity of the leftside.
     * @param i_uR particles velocity of the right side.
     * @param o_waveSpeedL will be set to the speed of the wave propagating to the left.
     * @param o_waveSpeedR will be set to the speed of the wave propagating to the right.
     **/
    static void waveSpeeds( t_real   i_hL,
                            t_real   i_hR,
                            t_real   i_uL,
                            t_real   i_uR,
                            t_real & o_waveSpeedL,
                            t_real & o_waveSpeedR );

    /**
     * Computes the wave flux.
     *
     * @param i_hL height of the left side.
     * @param i_hR height of the right side.
     * @param i_huL momentum of the left side.
     * @param i_huR momentum of the right side.
     * @param i_bL bathymetry of the left side.
     * @param i_bR bathymetry of the right side.
     * @param i_waveSpeedL speed of the wave propagating to the left.
     * @param i_waveSpeedR speed of the wave propagating to the right.
     * @param o_fluxL will be set to the flux of the wave propagating to the left.
     * @param o_fluxR will be set to the flux of the wave propagating to the right.
     **/
    static void waveFlux( t_real   i_hL,
                               t_real   i_hR,
                               t_real   i_huL,
                               t_real   i_huR,
                               t_real   i_bL,
                               t_real   i_bR,
                               t_real & o_fluxL,
                               t_real & o_fluxR );

  public:
    /**
     * Computes the net-updates.
     *
     * @param i_hL height of the left side.
     * @param i_hR height of the right side.
     * @param i_huL momentum of the left side.
     * @param i_huR momentum of the right side.
     * @param i_bL bathymetry of the left side.
     * @param i_bR bathymetry of the right side.
     * @param o_netUpdateL will be set to the net-updates for the left side; 0: height, 1: momentum.
     * @param o_netUpdateR will be set to the net-updates for the right side; 0: height, 1: momentum.
     **/
    static void netUpdates( t_real i_hL,
                            t_real i_hR,
                            t_real i_huL,
                            t_real i_huR,
                            t_real i_bL,
                            t_real i_bR,
                            t_real o_netUpdateL[2],
                            t_real o_netUpdateR[2] );
};

#endif
