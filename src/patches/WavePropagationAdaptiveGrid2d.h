/**
 * @author
 *
 * @section DESCRIPTION
 * Adaptive Mesh Refinement 2D wave propagation solver.
 **/
#ifndef TSUNAMI_LAB_PATCHES_WAVE_PROPAGATION_ADAPTIVE_GRID_2D
#define TSUNAMI_LAB_PATCHES_WAVE_PROPAGATION_ADAPTIVE_GRID_2D

#include "../constants.h"
#include "WavePropagation.h"
#include "WavePropagation2d.h"
#include "../setups/Setup.h"
#include <map>
#include <set>
#include <array>
#include <vector>

namespace tsunami_lab {
  namespace patches {
    class WavePropagationAdaptiveGrid2d : public WavePropagation {
    private:
      //! Coarse base grid
      WavePropagation2d* m_coarseGrid = nullptr;
      
      //! Fine grids for different refinement levels
      std::map<t_idx, WavePropagation2d*> m_fineGrids;
      
      //! Refinement map: (ix, iy) -> refinement factor
      std::map<std::pair<t_idx, t_idx>, t_idx> m_refinementMap;

      //! Bounding boxes for each refinement level: level -> [minX, minY, maxX, maxY]
      std::map<t_idx, std::array<t_idx, 4>> m_refinedBounds;
      
      //! Domain dimensions
      t_idx m_nCellsX = 0;
      t_idx m_nCellsY = 0;
      
      //! Boundary conditions
      bool m_leftReflecting = false;
      bool m_rightReflecting = false;
      bool m_bottomReflecting = false;
      bool m_topReflecting = false;
      
      /**
       * Checks if a coarse cell is refined
       **/
      bool isRefined(t_idx i_ix, t_idx i_iy) const;
      
      /**
       * Maps coarse (ix, iy) to fine grid coordinates
       **/
      void coarseToFineIndices(t_idx i_coarseX, t_idx i_coarseY, t_idx i_refinement,
                               t_idx& o_fineX, t_idx& o_fineY) const;
      
      /**
       * Interpolate coarse boundaries to fine ghost cells
       **/
      void interpolateBoundaries(t_idx i_refinement);
      
      /**
       * Restrict fine interior cells back to coarse grid
       **/
      void restrictBoundary(t_idx i_refinement);

    public:
      /**
       * @param i_nCellsX coarse grid cells in x-direction
       * @param i_nCellsY coarse grid cells in y-direction
       **/
      WavePropagationAdaptiveGrid2d(t_idx i_nCellsX, t_idx i_nCellsY);
      
      /**
       * Destructor
       **/
      ~WavePropagationAdaptiveGrid2d() override;
      
      /**
       * Set refinement map from setup
       * @param i_refinementMap map of (ix, iy) -> refinement factor
       **/
      void setRefinementMap(const std::map<std::pair<t_idx, t_idx>, t_idx>& i_refinementMap);
      
      /**
       * @param i_scaling scaling of the time step for coarse grid
       **/
      void timeStep(t_real i_scaling) override;
      
      /**
       * Sets ghost cells with given boundary conditions
       **/
      void setGhostCells(bool i_leftReflecting,
                         bool i_rightReflecting,
                         bool i_bottomReflecting = false,
                         bool i_topReflecting = false) override;
      
      /**
       * Gets stride in y-direction
       **/
      t_idx getStride() override;
      
      /**
       * Gets water heights from coarse grid
       **/
      t_real const * getHeight() override;
      
      /**
       * Gets momentum in x-direction from coarse grid
       **/
      t_real const * getMomentumX() override;
      
      /**
       * Gets momentum in y-direction from coarse grid
       **/
      t_real const * getMomentumY() override;
      
      /**
       * Gets bathymetry from coarse grid
       **/
      t_real const * getBathymetry() override;
      
      /**
       * Sets height at coarse cell (routes to appropriate grid)
       **/
      void setHeight(t_idx i_ix, t_idx i_iy, t_real i_h, setups::Setup* i_setup) override;
      
      /**
       * Sets momentum x at coarse cell (routes to appropriate grid)
       **/
      void setMomentumX(t_idx i_ix, t_idx i_iy, t_real i_hu) override;
      
      /**
       * Sets momentum y at coarse cell (routes to appropriate grid)
       **/
      void setMomentumY(t_idx i_ix, t_idx i_iy, t_real i_hv) override;
      
      /**
       * Sets bathymetry at coarse cell (routes to appropriate grid)
       **/
      void setBathymetry(t_idx i_ix, t_idx i_iy, t_real i_b, setups::Setup* i_setup) override;

      /**
       * Gets refinement factor for a coarse cell
       **/
      t_idx getRefinement(t_idx i_ix, t_idx i_iy) const;

      void exportUniformGrid(
          t_idx i_maxResolution,
          std::vector<t_real>& o_b,
          std::vector<t_real>& o_h,
          std::vector<t_real>& o_hu,
          std::vector<t_real>& o_hv
      ) const;

    };
  }
}

#endif