/**
 * @author Alexander Breuer (alex.breuer AT uni-jena.de)
 *
 * @section DESCRIPTION
 * One-dimensional wave propagation patch.
 **/
#include "WavePropagation1d.h"
#include "../solvers/Roe.h"
#include "../solvers/f_solver.h"
#include <iostream>
#include <string>

tsunami_lab::patches::WavePropagation1d::WavePropagation1d( t_idx i_nCells, const std::string &i_solver_model ) {
  m_step = 0;
  
  m_nCells = i_nCells;
  //solver = i_solver_model;

  m_solver = i_solver_model;
  //std::string m_solver = i_solver_model;

  // allocate memory including a single ghost cell on each side (height, momentum)
  for( unsigned short l_st = 0; l_st < 2; l_st++ ) {
    m_h[l_st] = new t_real[  m_nCells + 2 ];
    m_hu[l_st] = new t_real[ m_nCells + 2 ];
  }

  // allocate single bathymetry array, floor height does not change and therefore does not need to be double buffered
  m_b = new t_real[m_nCells + 2];

  // init to zero
  for( unsigned short l_st = 0; l_st < 2; l_st++ ) {
    for( t_idx l_ce = 0; l_ce < m_nCells + 2; l_ce++ ) {
      m_h[l_st][l_ce] = 0;
      m_hu[l_st][l_ce] = 0;
    }
  }

  // init bathymetry to zero 
  for( t_idx l_ce = 0; l_ce < m_nCells + 2; l_ce++ ) {
    m_b[l_ce] = 0;
  }

}

// dESTRUCTOR
tsunami_lab::patches::WavePropagation1d::~WavePropagation1d() {
  for( unsigned short l_st = 0; l_st < 2; l_st++ ) {
    delete[] m_h[l_st];
    delete[] m_hu[l_st];
  }
  delete[] m_b;
}

void tsunami_lab::patches::WavePropagation1d::timeStep( t_real i_scaling ) {
  // pointers to old and new data
  t_real * l_hOld = m_h[m_step];
  t_real * l_huOld = m_hu[m_step];

  m_step = (m_step+1) % 2;
  t_real * l_hNew =  m_h[m_step];
  t_real * l_huNew = m_hu[m_step];

  // init new cell quantities
  for( t_idx l_ce = 1; l_ce < m_nCells+1; l_ce++ ) {
    l_hNew[l_ce] = l_hOld[l_ce];
    l_huNew[l_ce] = l_huOld[l_ce];
  }

  // iterate over edges and update with Riemann solutions
  for( t_idx l_ed = 0; l_ed < m_nCells+1; l_ed++ ) {
    // determine left and right cell-id
    t_idx l_ceL = l_ed;
    t_idx l_ceR = l_ed+1;

    // compute net-updates
    t_real l_netUpdates[2][2];

    // initialize strings
    //std::string solver = m_solver;
    std::string t = "roe";

    // check which solver to use
    if( m_solver == "roe" ){
    solvers::Roe::netUpdates( l_hOld[l_ceL],
                              l_hOld[l_ceR],
                              l_huOld[l_ceL],
                              l_huOld[l_ceR],
                              l_netUpdates[0],
                              l_netUpdates[1] );
    } 
    // currently uses f-wave if not specifically told otherwise, now with bathymetry
    else {
    solvers::f_wave::netUpdates(  l_hOld[l_ceL],
                                  l_hOld[l_ceR],
                                  l_huOld[l_ceL],
                                  l_huOld[l_ceR],
                                  m_b[l_ceL],     
                                  m_b[l_ceR],
                                  l_netUpdates[0],
                                  l_netUpdates[1] );
    }

    // update the cells' quantities
    l_hNew[l_ceL]  -= i_scaling * l_netUpdates[0][0];
    l_huNew[l_ceL] -= i_scaling * l_netUpdates[0][1];

    l_hNew[l_ceR]  -= i_scaling * l_netUpdates[1][0];
    l_huNew[l_ceR] -= i_scaling * l_netUpdates[1][1];
  }
}

// WavePropagation1d.cpp – just add the two ignored parameters
void tsunami_lab::patches::WavePropagation1d::setGhostCells( bool i_leftReflecting,
                                                             bool i_rightReflecting,
                                                             bool ,     // unused bottomReflecting
                                                             bool  ) {  // unused topReflecting
  t_real * l_h  = m_h[m_step];
  t_real * l_hu = m_hu[m_step];

  // left boundary
  l_h[0]  = l_h[1];
  l_hu[0] = i_leftReflecting ? -l_hu[1] : l_hu[1];
  m_b[0]  = m_b[1];

  // right boundary
  l_h[m_nCells+1]  = l_h[m_nCells];
  l_hu[m_nCells+1] = i_rightReflecting ? -l_hu[m_nCells] : l_hu[m_nCells];
  m_b[m_nCells+1]  = m_b[m_nCells];
}