/**
 * @author 
 *
 * @section DESCRIPTION
 * F-Wave solver for the shallow water equations.
 **/
#include "f_solver.h"
#include <cmath>

// exactly the same as in the Roe Riemann solver
void tsunami_lab::solvers::f_wave::waveSpeeds( t_real   i_hL,
                                            t_real   i_hR,
                                            t_real   i_uL,
                                            t_real   i_uR,
                                            t_real & o_waveSpeedL,
                                            t_real & o_waveSpeedR ) {
  // pre-compute square-root ops
  t_real l_hSqrtL = std::sqrt( i_hL );
  t_real l_hSqrtR = std::sqrt( i_hR );

  // compute Roe averages
  t_real l_hRoe = 0.5f * ( i_hL + i_hR );
  t_real l_uRoe = l_hSqrtL * i_uL + l_hSqrtR * i_uR;
  l_uRoe /= l_hSqrtL + l_hSqrtR;

  // compute wave speeds
  t_real l_ghSqrtRoe = m_gSqrt * std::sqrt( l_hRoe );
  o_waveSpeedL = l_uRoe - l_ghSqrtRoe;
  o_waveSpeedR = l_uRoe + l_ghSqrtRoe;
}

// replacing waveStrength with flux function
void tsunami_lab::solvers::f_wave::waveFlux( t_real   i_hL,
                                               t_real   i_hR,
                                               t_real   i_huL,
                                               t_real   i_huR,
                                               t_real   i_bL,
                                               t_real   i_bR,
                                               t_real & o_fluxdiffH,
                                               t_real & o_fluxdiffHu ) {
  // compute particle velocities 
  t_real l_uL = i_huL / i_hL;
  t_real l_uR = i_huR / i_hR;

  // compute two flux vectors (like in source (1.1))
  t_real l_fluxL[2], l_fluxR[2];

  // left vector
  l_fluxL[0] = i_huL;  
  l_fluxL[1] = i_huL * l_uL + 0.5f * m_g * i_hL * i_hL;     // h*u^2 + 1/2*g*h^2

  // right vector
  l_fluxR[0] = i_huR;
  l_fluxR[1] = i_huR * l_uR + 0.5f * m_g * i_hR * i_hR;     // h*u^2 + 1/2*g*h^2

  // calculate effect of the bathymetry:
  t_real l_dx[2];
  l_dx[0] = 0;
  l_dx[1] = -m_g * (i_bR - i_bL) * (i_hL + i_hR) * 1/2; 

  // calculate differernces (essentially Δf)
  o_fluxdiffH = l_fluxR[0] - l_fluxL[0] - l_dx[0];   //subtracting l_dx to include bathymetry effect
  o_fluxdiffHu = l_fluxR[1] - l_fluxL[1] - l_dx[1]; 
}

// actual changes in calculation
void tsunami_lab::solvers::f_wave::netUpdates( t_real i_hL,
                                            t_real i_hR,
                                            t_real i_huL,
                                            t_real i_huR,
                                            t_real i_bL,
                                            t_real i_bR,
                                            t_real o_netUpdateL[2],
                                            t_real o_netUpdateR[2] ) {
  // compute particle velocities 
  t_real l_uL = i_huL / i_hL;
  t_real l_uR = i_huR / i_hR;

  // compute wave speeds
  t_real l_sL = 0;
  t_real l_sR = 0;

  waveSpeeds( i_hL,
              i_hR,
              l_uL,
              l_uR,
              l_sL,
              l_sR );

  // compute wave flux
  t_real l_fdH = 0;
  t_real l_fdHu = 0;

  waveFlux( i_hL,
            i_hR,
            i_huL,
            i_huR,
            i_bL,
            i_bR,
            l_fdH,
            l_fdHu );

  // calculate eigenvector α[2] in as in αL = (matrix inversion prefix) * (sR * Δf(h) - 1 * Δf(hu)) and αR =  (matrix inversion prefix) * (-sL * Δf(h) + 1 * Δf(hu))
  // first calculate inverted R (only prefix)
  t_real l_detInv = 1.0f / (l_sR - l_sL);
  // second calculate αL and αR
  t_real l_alphaL = l_detInv * ( l_sR * l_fdH - l_fdHu );
  t_real l_alphaR = l_detInv * (-l_sL * l_fdH + l_fdHu );
  
  //calculate the actual waves Zleft and Zright because i don't fully understand the summations from (1.3.4)
  // waves = α_p * r_p
  t_real l_zL[2];
  l_zL[0] = l_alphaL;         
  l_zL[1] = l_alphaL * l_sL;  

  t_real l_zR[2];
  l_zR[0] = l_alphaR;         
  l_zR[1] = l_alphaR * l_sR;  

  // set net-updates depending on direction (A-ΔQ is left and A+ΔQ is right)
  for( unsigned short l_qt = 0; l_qt < 2; l_qt++ ) {
    // init
    o_netUpdateL[l_qt] = 0;
    o_netUpdateR[l_qt] = 0;

    // speed goes from left-to-right and negative speed means waves going right-to-left
    // 1st wave (from left column of water)
    if( l_sL < 0 ) {      
      o_netUpdateL[l_qt] += l_zL[l_qt];   // A-ΔQ or left
    }
    else {
      o_netUpdateR[l_qt] += l_zL[l_qt];   // A+ΔQ or right
    }

    // 2nd wave (from right column of water)
    if( l_sR > 0 ) {
      o_netUpdateR[l_qt] += l_zR[l_qt];   // A+ΔQ or right
    }
    else {
      o_netUpdateL[l_qt] += l_zR[l_qt];   // A-ΔQ or left
    }
  }
}