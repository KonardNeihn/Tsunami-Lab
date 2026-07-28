/**
 * @author 
 *
 * @section DESCRIPTION
 * F-Wave solver for the shallow water equations.
 **/
#include "f_solver.h"
#include <cmath>
#include <algorithm>

// exactly the same as in the Roe Riemann solver
void tsunami_lab::solvers::f_wave::waveSpeeds( t_real   i_hL,
                                               t_real   i_hR,
                                               t_real   i_uL,
                                               t_real   i_uR,
                                               t_real & o_waveSpeedL,
                                               t_real & o_waveSpeedR ) {
  t_real l_hSqrtL = std::sqrt( i_hL );
  t_real l_hSqrtR = std::sqrt( i_hR );

  t_real l_hRoe = 0.5f * ( i_hL + i_hR );
  t_real l_uRoe = ( l_hSqrtL * i_uL + l_hSqrtR * i_uR ) / ( l_hSqrtL + l_hSqrtR );

  t_real l_cRoe = m_gSqrt * std::sqrt( l_hRoe );
  t_real l_cL   = m_gSqrt * l_hSqrtL;   // sqrt(g*h_L)
  t_real l_cR   = m_gSqrt * l_hSqrtR;   // sqrt(g*h_R)

  // Einfeldt: die Roe-Werte allein schliessen die echten Wellen an starken
  // Tiefensprungen nicht ein -> zusaetzlich gegen die Zellgeschwindigkeiten
  // absichern, sonst wird die CFL lokal verletzt.
  o_waveSpeedL = std::min( i_uL - l_cL, l_uRoe - l_cRoe );
  o_waveSpeedR = std::max( i_uR + l_cR, l_uRoe + l_cRoe );
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

void tsunami_lab::solvers::f_wave::netUpdates( t_real i_hL,  t_real i_hR,
                                               t_real i_huL, t_real i_huR,
                                               t_real i_bL,  t_real i_bR,
                                               t_real o_netUpdateL[2],
                                               t_real o_netUpdateR[2] ) {

  o_netUpdateL[0] = 0; o_netUpdateL[1] = 0;
  o_netUpdateR[0] = 0; o_netUpdateR[1] = 0;

  bool l_dryL = !( i_hL > m_dryThreshold );
  bool l_dryR = !( i_hR > m_dryThreshold );

  // land-land: skip
  if( l_dryL && l_dryR ) return;            

  // mirror wet side as wall
  if( l_dryL ) { i_hL = i_hR; i_huL = -i_huR; i_bL = i_bR; }
  if( l_dryR ) { i_hR = i_hL; i_huR = -i_huL; i_bR = i_bL; }

  // clamp velocity
  const t_real l_uMax = 50.0f;
  t_real l_huL = std::max( -l_uMax * i_hL, std::min( l_uMax * i_hL, i_huL ) );
  t_real l_huR = std::max( -l_uMax * i_hR, std::min( l_uMax * i_hR, i_huR ) );

  t_real l_uL = l_huL / i_hL;
  t_real l_uR = l_huR / i_hR;

  // wave speeds
  t_real l_sL = 0, l_sR = 0;
  waveSpeeds( i_hL, i_hR, l_uL, l_uR, l_sL, l_sR );

  // flux difference
  t_real l_fdH = 0, l_fdHu = 0;
  waveFlux( i_hL, i_hR, l_huL, l_huR, i_bL, i_bR, l_fdH, l_fdHu );

  // wave strengths
  t_real l_detInv = 1.0f / (l_sR - l_sL);
  t_real l_alphaL = l_detInv * ( l_sR * l_fdH - l_fdHu );
  t_real l_alphaR = l_detInv * (-l_sL * l_fdH + l_fdHu );

  t_real l_zL[2] = { l_alphaL, l_alphaL * l_sL };
  t_real l_zR[2] = { l_alphaR, l_alphaR * l_sR };

  // upwind waves
  if( l_sL < 0 ) { o_netUpdateL[0] += l_zL[0]; o_netUpdateL[1] += l_zL[1]; }
  else           { o_netUpdateR[0] += l_zL[0]; o_netUpdateR[1] += l_zL[1]; }

  if( l_sR > 0 ) { o_netUpdateR[0] += l_zR[0]; o_netUpdateR[1] += l_zR[1]; }
  else           { o_netUpdateL[0] += l_zR[0]; o_netUpdateL[1] += l_zR[1]; }

  // no update on dry side
  if( l_dryL ) { o_netUpdateL[0] = 0; o_netUpdateL[1] = 0; }
  if( l_dryR ) { o_netUpdateR[0] = 0; o_netUpdateR[1] = 0; }
}