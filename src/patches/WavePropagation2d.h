/**
 * @author 
 *
 * @section DESCRIPTION
 * Two-dimensional wave propagation patch (unsplit method).
 **/
#ifndef TSUNAMI_LAB_PATCHES_WAVE_PROPAGATION_2D
#define TSUNAMI_LAB_PATCHES_WAVE_PROPAGATION_2D

#include "WavePropagation.h"
#include <iostream>

namespace tsunami_lab {
  namespace patches {
    class WavePropagation2d;
  }
}

class tsunami_lab::patches::WavePropagation2d: public WavePropagation {
  private:
    //! current step which indicates the active values in the arrays below
    unsigned short m_step = 0;

    //! number of cells in x-direction discretizing the computational domain
    t_idx m_nCellsX = 0;

    //! number of cells in y-direction discretizing the computational domain
    t_idx m_nCellsY = 0;

    //! water heights for the current and next time step for all cells
    //! layout: (m_nCellsX + 2) * (m_nCellsY + 2), row-major with y as the outer index
    t_real * m_h[2] = { nullptr, nullptr };

    //! momentum in x-direction for the current and next time step for all cells
    t_real * m_hu[2] = { nullptr, nullptr };

    //! momentum in y-direction for the current and next time step for all cells
    t_real * m_hv[2] = { nullptr, nullptr };

    //! bathymetry values for all cells (single buffer – floor height does not change)
    t_real * m_b = nullptr;

    //! temporary static threshold for dry-state purposes
    static constexpr t_real m_dryThreshold = 1e-4;

    /**
     * Computes the flat 1-D array index for cell (i_ix, i_iy) in the
     * padded (ghost-cell) grid of size (m_nCellsX+2) x (m_nCellsY+2).
     *
     * @param i_ix  x-index (0 = left ghost cell, 1..m_nCellsX = interior, m_nCellsX+1 = right ghost)
     * @param i_iy  y-index (0 = bottom ghost cell, 1..m_nCellsY = interior, m_nCellsY+1 = top ghost)
     **/
    t_idx idx( t_idx i_ix, t_idx i_iy ) const {
      return i_iy * (m_nCellsX + 2) + i_ix;
    }

  public:
    /**
     * Constructs the 2d wave propagation solver.
     *
     * @param i_nCellsX number of cells in x-direction.
     * @param i_nCellsY number of cells in y-direction.
     **/
    WavePropagation2d( t_idx i_nCellsX,
                       t_idx i_nCellsY );

    /**
     * Destructor which frees all allocated memory.
     **/
    ~WavePropagation2d() override;

    /**
     * Performs a single time step using the unsplit method.
     *
     * @param i_scaling scaling of the time step (dt / dx == dt / dy assumed uniform).
     **/
    void timeStep( t_real i_scaling ) override;

    /**
     * Sets the values of the ghost cells according to outflow (transmissive)
     * or reflecting boundary conditions.
     *
     * @param i_leftReflecting   reflecting condition on left   (x = 0)   boundary.
     * @param i_rightReflecting  reflecting condition on right  (x = max) boundary.
     * @param i_bottomReflecting reflecting condition on bottom (y = 0)   boundary.
     * @param i_topReflecting    reflecting condition on top    (y = max) boundary.
     **/
    void setGhostCells( bool i_leftReflecting,
                        bool i_rightReflecting,
                        bool i_bottomReflecting = false,
                        bool i_topReflecting    = false ) override;

    /**
     * Gets the stride in y-direction (number of entries per row in the flat arrays).
     *
     * @return stride in y-direction.
     **/
    t_idx getStride() override{
      return m_nCellsX + 2;
    }

    /**
     * Gets cells' water heights (pointer to first interior cell).
     *
     * @return water heights.
     **/
    t_real const * getHeight() override {
      // skip the bottom ghost row and the left ghost column of the first interior row
      return m_h[m_step] + idx(1, 1);
    }

    /**
     * Gets cells' floor heights (pointer to first interior cell).
     *
     * @return floor heights.
     **/
    t_real const * getBathymetry() override {
      return m_b + idx(1, 1);
    }

    /**
     * Gets the cells' momentum in x-direction (pointer to first interior cell).
     *
     * @return momentum in x-direction.
     **/
    t_real const * getMomentumX() override {
      return m_hu[m_step] + idx(1, 1);
    }

    /**
     * Gets the cells' momentum in y-direction (pointer to first interior cell).
     *
     * @return momentum in y-direction.
     **/
    t_real const * getMomentumY() override {
      return m_hv[m_step] + idx(1, 1);
    }

    /**
     * Sets the height of the cell to the given value.
     *
     * @param i_ix id of the cell in x-direction.
     * @param i_iy id of the cell in y-direction.
     * @param i_h  water height.
     **/
    void setHeight( t_idx  i_ix,
                    t_idx  i_iy,
                    t_real i_h,
                  [[maybe_unused]] setups::Setup* i_setup = nullptr ) override {
      m_h[m_step][idx(i_ix + 1, i_iy + 1)] = i_h;
    }

    /**
     * Sets the momentum in x-direction to the given value.
     *
     * @param i_ix  id of the cell in x-direction.
     * @param i_iy  id of the cell in y-direction.
     * @param i_hu  momentum in x-direction.
     **/
    void setMomentumX( t_idx  i_ix,
                       t_idx  i_iy,
                       t_real i_hu ) override {
      m_hu[m_step][idx(i_ix + 1, i_iy + 1)] = i_hu;
    }

    /**
     * Sets the momentum in y-direction to the given value.
     *
     * @param i_ix  id of the cell in x-direction.
     * @param i_iy  id of the cell in y-direction.
     * @param i_hv  momentum in y-direction.
     **/
    void setMomentumY( t_idx  i_ix,
                       t_idx  i_iy,
                       t_real i_hv ) override {
      m_hv[m_step][idx(i_ix + 1, i_iy + 1)] = i_hv;
    }

    /**
     * Sets the bathymetry of the cell to the given value.
     *
     * @param i_ix id of the cell in x-direction.
     * @param i_iy id of the cell in y-direction.
     * @param i_b  bathymetry value.
     **/
    void setBathymetry( t_idx  i_ix,
                        t_idx  i_iy,
                        t_real i_b,
                        [[maybe_unused]] setups::Setup* i_setup = nullptr ) override {
      m_b[idx(i_ix + 1, i_iy + 1)] = i_b;
    }
};

#endif