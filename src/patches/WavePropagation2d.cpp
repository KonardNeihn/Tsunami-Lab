/**
 * @author 
 *
 * @section DESCRIPTION
 * Two-dimensional wave propagation patch (unsplit method).
 **/
#include "WavePropagation2d.h"
#include "../solvers/f_solver.h"
#include <iostream>
#include <omp.h>

tsunami_lab::patches::WavePropagation2d::WavePropagation2d( t_idx i_nCellsX,
                                                             t_idx i_nCellsY ) {
  m_nCellsX = i_nCellsX;
  m_nCellsY = i_nCellsY;

  t_idx l_size = (m_nCellsX + 2) * (m_nCellsY + 2);

  // allocate double-buffered height and momentum arrays for both directions
  for( unsigned short l_st = 0; l_st < 2; l_st++ ) {
    m_h[l_st]  = new t_real[l_size];
    m_hu[l_st] = new t_real[l_size];
    m_hv[l_st] = new t_real[l_size];
  }

  // allocate single bathymetry array (floor height is time-invariant)
  m_b = new t_real[l_size];

  // initialise all arrays to zero, now with the same scheduling as the simulation itself to use the First-Touch-Policy correctly
  for( unsigned short l_st = 0; l_st < 2; l_st++ ) {
    #pragma omp parallel for schedule(static)
    for( t_idx l_iy = 0; l_iy < m_nCellsY + 2; l_iy++ ) {
      for( t_idx l_ix = 0; l_ix < m_nCellsX + 2; l_ix++ ) {
        t_idx l_ce = idx(l_ix, l_iy);
        m_h[l_st][l_ce]  = 0;  // First Touch by threads
        m_hu[l_st][l_ce] = 0;
        m_hv[l_st][l_ce] = 0;
      }
    }
  }

  // bathymetry values also need the same initialization
  #pragma omp parallel for schedule(static)
  for( t_idx l_iy = 0; l_iy < m_nCellsY + 2; l_iy++ ) {
    for( t_idx l_ix = 0; l_ix < m_nCellsX + 2; l_ix++ ) {
      t_idx l_ce = idx(l_ix, l_iy);
      m_b[l_ce] = 0;
    }
  }
}

// free space after calculation
tsunami_lab::patches::WavePropagation2d::~WavePropagation2d() {
  for( unsigned short l_st = 0; l_st < 2; l_st++ ) {
    delete[] m_h[l_st];
    delete[] m_hu[l_st];
    delete[] m_hv[l_st];
  }
  delete[] m_b;
}

void tsunami_lab::patches::WavePropagation2d::timeStep( t_real i_scaling ) {
  // Pointers to old (current) and new (next) data buffers
  t_real * l_hOld  = m_h[m_step];
  t_real * l_huOld = m_hu[m_step];
  t_real * l_hvOld = m_hv[m_step];

  m_step = (m_step + 1) % 2;  // % 2 ist probably nicht notwendig

  t_real * l_hNew  = m_h[m_step];
  t_real * l_huNew = m_hu[m_step];
  t_real * l_hvNew = m_hv[m_step];

  // Copy interior cell values from old to new buffer as the starting point.
  // Ghost cells are excluded since they are set by setGhostCells().
  #pragma omp parallel for schedule(static)   // parallelization
  for( t_idx l_iy = 1; l_iy <= m_nCellsY; l_iy++ ) {

    // #pragma omp parallel for
    for( t_idx l_ix = 1; l_ix <= m_nCellsX; l_ix++ ) {
      t_idx l_ce   = idx(l_ix, l_iy);
      l_hNew[l_ce]  = l_hOld[l_ce];
      l_huNew[l_ce] = l_huOld[l_ce];
      l_hvNew[l_ce] = l_hvOld[l_ce];
    }
  }

  // X-sweep: iterates over all vertical edges  x_{i-1/2, j}
  // Each edge is shared by cell (i-1, j) on the left and cell (i, j) on the right.
  // The f-wave solver does not change the y-direction-momentum.
  #pragma omp parallel for schedule(static)   // parallelization
  for( t_idx l_iy = 1; l_iy <= m_nCellsY; l_iy++ ) {
    // l_ix = 0 gives the edge between the left ghost-cell and the first interior cell and l_ix = m_nCellsX gives the edge between the last and the right ghost-cell.
    
    // #pragma omp parallel for
    for( t_idx l_ix = 0; l_ix <= m_nCellsX; l_ix++ ) {
      t_idx l_ceL = idx(l_ix,     l_iy);   // left  cell of vertical edge
      t_idx l_ceR = idx(l_ix + 1, l_iy);   // right cell of vertical edge

      // initialize new netUpdate array for both collumn-values
      t_real l_netUpdates[2][2];

      /*
      if (l_hOld[l_ceL] <= m_b[l_ceL]) {
        std::cout << "Water Height lower then Bathymetry at (" << l_ix << ", " << l_iy << ") with h = " << l_hOld[l_ceL] << " and b = " << m_b[l_ceL] << std::endl;
      }
      */

      // netUpdate mit 
      solvers::f_wave::netUpdates(
                                    l_hOld[l_ceL],
                                    l_hOld[l_ceR],
                                    l_huOld[l_ceL],
                                    l_huOld[l_ceR],
                                    m_b[l_ceL],
                                    m_b[l_ceR],
                                    l_netUpdates[0],
                                    l_netUpdates[1]
      );

      if( l_ix > 0 ) {
        l_hNew[l_ceL]  -= i_scaling * l_netUpdates[0][0];
        l_huNew[l_ceL] -= i_scaling * l_netUpdates[0][1];
      }

      if( l_ix < m_nCellsX ) {
        l_hNew[l_ceR]  -= i_scaling * l_netUpdates[1][0];
        l_huNew[l_ceR] -= i_scaling * l_netUpdates[1][1];
      }
      /*
        if( l_hNew[l_ceL] <= m_b[l_ceL] ) {
          std::cout << "BAD UPDATE LEFT at "
          << l_ix << " " << l_iy
          << " hNew=" << l_hNew[l_ceL]
          << " b=" << m_b[l_ceL]
          << std::endl;
      }*/
    }
  }

  // Y-sweep: iterate over all horizontal edges y_{i, j-1/2}
  // Each edge is shared by cell (i, j-1) on the bottom and cell (i, j) on top.
  // The f-wave solver is applied to a transposed version of the problem:
  //   h stays h, the "x-momentum" hu is now the "y-momentum" hv.
  //
  // IMPORTANT: we read from the OLD buffers (l_hOld / l_hvOld), not from the intermediate result of the x-sweep.
  // Both sweeps therefore see the same Q^n state, and their net-updates are added independently into l_hNew / l_hvNew.
  #pragma omp parallel for schedule(static) 
  for( t_idx l_iy = 0; l_iy <= m_nCellsY; l_iy++ ) {
    //#pragma omp parallel for schedule(static)
    for( t_idx l_ix = 1; l_ix <= m_nCellsX; l_ix++ ) {
      t_idx l_ceB = idx(l_ix, l_iy);       // bottom cell of horizontal edge
      t_idx l_ceT = idx(l_ix, l_iy + 1);   // top    cell of horizontal edge

      t_real l_netUpdates[2][2];

      // Pass hv as the "normal" momentum to the 1-D solver.
      // Both h and hv are taken from the OLD (Q^n) buffer.
      solvers::f_wave::netUpdates(
                                    l_hOld[l_ceB],
                                    l_hOld[l_ceT],
                                    l_hvOld[l_ceB], 
                                    l_hvOld[l_ceT],
                                    m_b[l_ceB], 
                                    m_b[l_ceT],
                                    l_netUpdates[0],
                                    l_netUpdates[1]
      );

      // new atomic operators to prevent race-conditions
      if( l_iy > 0 ) {
        #pragma omp atomic
        l_hNew[l_ceB]  -= i_scaling * l_netUpdates[0][0];
        #pragma omp atomic
        l_hvNew[l_ceB] -= i_scaling * l_netUpdates[0][1];
      }

      if( l_iy < m_nCellsY ) {
        #pragma omp atomic
        l_hNew[l_ceT]  -= i_scaling * l_netUpdates[1][0];
        #pragma omp atomic
        l_hvNew[l_ceT] -= i_scaling * l_netUpdates[1][1];
      }

      // B+ ΔQ_{i,j-1/2} for updates to the bottom cell

      // B- ΔQ_{i,j+1/2} for updates to the top cell
      
    }
  }
  // CLAMP: after both sweeps, zero out any cells that went dry
  #pragma omp parallel for schedule(static)   // parallelization
  for( t_idx l_iy = 1; l_iy <= m_nCellsY; l_iy++ ) {

    // #pragma omp parallel for
    for( t_idx l_ix = 1; l_ix <= m_nCellsX; l_ix++ ) {
      t_idx l_ce = idx(l_ix, l_iy);
      if( l_hNew[l_ce] <= m_dryThreshold ) {
        l_hNew[l_ce]  = 0;
        l_huNew[l_ce] = 0;
        l_hvNew[l_ce] = 0;
      }
    }
  }
}

// the ghost-cells are more complicated now, since it iessentially an entire outer layer around the simulated space.
void tsunami_lab::patches::WavePropagation2d::setGhostCells( bool i_leftReflecting,
                                                             bool i_rightReflecting,
                                                             bool i_bottomReflecting,
                                                             bool i_topReflecting ) {
  t_real * l_h  = m_h[m_step];
  t_real * l_hu = m_hu[m_step];
  t_real * l_hv = m_hv[m_step];

  // Left ghost column (ix = 0) mirrors first interior column (ix = 1)
  for( t_idx l_iy = 0; l_iy < m_nCellsY + 2; l_iy++ ) {
    t_idx l_ghost = idx(0, l_iy);
    t_idx l_inner = idx(1, l_iy);

    l_h[l_ghost]  = l_h[l_inner];
    l_hu[l_ghost] = i_leftReflecting ? -l_hu[l_inner] : l_hu[l_inner];
    l_hv[l_ghost] = l_hv[l_inner];
    m_b[l_ghost]  = m_b[l_inner];
  }

  // Right ghost column (ix = m_nCellsX+1) mirrors last interior column (ix = m_nCellsX)
  for( t_idx l_iy = 0; l_iy < m_nCellsY + 2; l_iy++ ) {
    t_idx l_ghost = idx(m_nCellsX + 1, l_iy);
    t_idx l_inner = idx(m_nCellsX,     l_iy);

    l_h[l_ghost]  = l_h[l_inner];
    l_hu[l_ghost] = i_rightReflecting ? -l_hu[l_inner] : l_hu[l_inner];
    l_hv[l_ghost] = l_hv[l_inner];
    m_b[l_ghost]  = m_b[l_inner];
  }

  // Bottom ghost row (iy = 0) irrors first interior row (iy = 1)
  for( t_idx l_ix = 0; l_ix < m_nCellsX + 2; l_ix++ ) {
    t_idx l_ghost = idx(l_ix, 0);
    t_idx l_inner = idx(l_ix, 1);

    l_h[l_ghost]  = l_h[l_inner];
    l_hu[l_ghost] = l_hu[l_inner];
    l_hv[l_ghost] = i_bottomReflecting ? -l_hv[l_inner] : l_hv[l_inner];
    m_b[l_ghost]  = m_b[l_inner];
  }

  // Top ghost row (iy = m_nCellsY+1) mirrors last interior row (iy = m_nCellsY)
  for( t_idx l_ix = 0; l_ix < m_nCellsX + 2; l_ix++ ) {
    t_idx l_ghost = idx(l_ix, m_nCellsY + 1);
    t_idx l_inner = idx(l_ix, m_nCellsY);

    l_h[l_ghost]  = l_h[l_inner];
    l_hu[l_ghost] = l_hu[l_inner];
    l_hv[l_ghost] = i_topReflecting ? -l_hv[l_inner] : l_hv[l_inner];
    m_b[l_ghost]  = m_b[l_inner];
  }
}