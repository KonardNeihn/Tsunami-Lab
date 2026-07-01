/**
 * @author Konard Neihn
 *
 * @section DESCRIPTION
 * IO class for writing simulation data with adaptive grid resolution to netCDF files.
 * Supports COARDS conventions for ParaView visualization.
 *
 **/
#ifndef TSUNAMI_LAB_IO_NETCDF_ADAPTIVE_GRID_H
#define TSUNAMI_LAB_IO_NETCDF_ADAPTIVE_GRID_H

#include <string>
#include <netcdf.h>
#include "../constants.h"
#include <vector>
#include <map>

namespace tsunami_lab {
  namespace io {
    class NetCdfAdaptiveGrid;
  }
}

/**
 * The file stores:
 *  - Coordinate variables: x, y (spatial), time (unlimited dimension)
 *  - Data variables: bathymetry (b), height (h), momentum_x (hu), momentum_y (hv)
 *  - Grid resolution levels for adaptive mesh
 *
 * Ghost cells are excluded from output by default.
 *
 * Typical usage:
 * @code
 *   tsunami_lab::io::NetCdfAdaptiveGrid l_writer(
 *       "output.nc",
 *       l_nx, l_ny,
 *       l_dxy,
 *       l_waveProp->getStride(),
 *       l_waveProp->getBathymetry(),
 *       l_gridResolutionLevels  // 2D vector with resolution levels for each cell
 *   );
 *
 *   // Inside the time loop:
 *   l_writer.write(
 *       l_simTime,
 *       l_waveProp->getHeight(),
 *       l_waveProp->getMomentumX(),
 *       l_waveProp->getMomentumY()
 *   );
 * @endcode
 */
class tsunami_lab::io::NetCdfAdaptiveGrid {
private:
    //! netCDF file id
    int m_ncId = -1;

    //! Dimension IDs: x, y, time, resolution_level
    int m_dimXId = -1;
    int m_dimYId = -1;
    int m_dimTimeId = -1;
    int m_dimResolutionId = -1;

    //! Variable IDs
    int m_varXId = -1;
    int m_varYId = -1;
    int m_varTimeId = -1;
    int m_varBId = -1;      // Bathymetry
    int m_varHId = -1;      // Water height
    int m_varHuId = -1;     // X-momentum
    int m_varHvId = -1;     // Y-momentum
    int m_varResolutionId = -1; // Grid resolution level

    //! Number of interior cells in x- and y-directions (ghost cells excluded)
    tsunami_lab::t_idx m_nx = 0;
    tsunami_lab::t_idx m_ny = 0;

    //! Stride of the solver arrays (interior + 2 ghost cells in x: m_nx + 2)
    tsunami_lab::t_idx m_stride = 0;

    //! Cell size in meters
    tsunami_lab::t_real m_dxy = 0.0;

    //! Maximum refinement level
    tsunami_lab::t_idx m_maxResolutionLevel = 0;

    //! Number of output cells in x and y after coarsening
    tsunami_lab::t_idx m_nxOut = 0;
    tsunami_lab::t_idx m_nyOut = 0;

    //! Current time step index (used as unlimited dimension record index)
    size_t m_timeStep = 0;

    //! Grid resolution levels for each cell (2D vector)
    std::vector<std::vector<tsunami_lab::t_idx>> m_gridResolutionLevels;

    //! True when the simulation is 2D (m_ny > 1)
    bool m_is2D = true;

    //! True if using checkpoint setup
    bool m_isCheckpoint = false;


    /**
     * Checks a netCDF return code and aborts with a descriptive message on error.
     *
     * @param i_status  Return code from a netCDF call.
     * @param i_context Error message context.
     */
    void checkNcErr(int i_status, const char* i_context) const;

public:
    /**
     * Opens (or creates) a netCDF output file and writes the static metadata plus the bathymetry variable.
     *
     * @param i_path               Path to the output .nc file (created or overwritten).
     * @param i_nx                 Number of interior cells in x-direction.
     * @param i_ny                 Number of interior cells in y-direction (use 1 for 1D).
     * @param i_dxy                Cell size [m] (uniform in x and y for base grid).
     * @param i_stride             Stride of the solver arrays (typically i_nx + 2).
     * @param i_b                  Pointer to the first *interior* bathymetry value.
     * @param i_gridResolution     2D vector with resolution levels for each cell.
     * @param i_isCheckpoint       True if this is a checkpoint file.
     */
    NetCdfAdaptiveGrid(
        const std::string& i_path,
        tsunami_lab::t_idx i_nx,
        tsunami_lab::t_idx i_ny,
        tsunami_lab::t_real i_dxy,
        tsunami_lab::t_idx i_stride,
        const tsunami_lab::t_real* i_b,
        const std::vector<std::vector<tsunami_lab::t_idx>>& i_gridResolution,
        bool i_isCheckpoint = false
    );

    // Flushes pending writes and closes the netCDF file.
    ~NetCdfAdaptiveGrid();

    /**
     * Appends one time record to the netCDF file.
     *
     * Writes the simulation time and the three prognostic fields (h, hu, hv) for all interior cells.
     * Ghost cells are skipped automatically.
     *
     * @param i_simTime  Physical simulation time [s] to record.
     * @param i_h        Pointer to the first interior water-height value.
     * @param i_hu       Pointer to the first interior x-momentum value.
     * @param i_hv       Pointer to the first interior y-momentum value (nullptr for 1D).
     */
    void write(
        tsunami_lab::t_real i_simTime,
        const tsunami_lab::t_real* i_h,
        const tsunami_lab::t_real* i_hu,
        const tsunami_lab::t_real* i_hv
    );
};

#endif // TSUNAMI_LAB_IO_NETCDF_ADAPTIVE_GRID_H