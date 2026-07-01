/**
 * @author Konard Neihn
 *
 * @section DESCRIPTION
 * COARDS-compliant netCDF writer for adaptive simulations.
 *
 * This class assumes that all simulation data has already been exported to a
 * uniform grid. It performs no interpolation or adaptive-grid logic itself;
 * it simply writes the supplied arrays to a netCDF file.
 */
#ifndef TSUNAMI_LAB_IO_NETCDF_ADAPTIVE_GRID_H
#define TSUNAMI_LAB_IO_NETCDF_ADAPTIVE_GRID_H

#include <string>
#include <vector>
#include <netcdf.h>

#include "../constants.h"

namespace tsunami_lab {
namespace io {

class NetCdfAdaptiveGrid {
  private:
    //! netCDF file id
    int m_ncId = -1;

    //! dimension ids
    int m_dimTimeId = -1;
    int m_dimXId    = -1;
    int m_dimYId    = -1;

    //! variable ids
    int m_varTimeId = -1;
    int m_varXId    = -1;
    int m_varYId    = -1;

    int m_varBId  = -1;
    int m_varHId  = -1;
    int m_varHuId = -1;
    int m_varHvId = -1;

    //! output grid size
    tsunami_lab::t_idx m_nxOut = 0;
    tsunami_lab::t_idx m_nyOut = 0;

    //! uniform cell size
    tsunami_lab::t_real m_dxyOut = 0;

    //! current output timestep
    size_t m_timeStep = 0;

    //! true for 2D simulations
    bool m_is2D = true;

    //! checkpoint output
    bool m_isCheckpoint = false;

    /**
     * Checks a netCDF return code.
     *
     * @param i_status return code.
     * @param i_context description.
     */
    void checkNcErr(
        int i_status,
        const char *i_context
    ) const;

  public:
    /**
     * Creates a new netCDF output file and writes all static information.
     *
     * Bathymetry must already be given on the final uniform output grid.
     *
     * @param i_path output file
     * @param i_nxOut number of output cells in x
     * @param i_nyOut number of output cells in y
     * @param i_dxyOut uniform output cell size
     * @param i_b bathymetry on the uniform output grid
     * @param i_isCheckpoint checkpoint mode
     */
    NetCdfAdaptiveGrid(
        const std::string &i_path,
        tsunami_lab::t_idx i_nxOut,
        tsunami_lab::t_idx i_nyOut,
        tsunami_lab::t_real i_dxyOut,
        const std::vector<tsunami_lab::t_real> &i_b,
        bool i_isCheckpoint = false
    );

    //! closes the file
    ~NetCdfAdaptiveGrid();

    /**
     * Writes one timestep.
     *
     * All arrays must already contain the fully expanded uniform grid.
     *
     * @param i_simTime simulation time
     * @param i_h water height
     * @param i_hu x-momentum
     * @param i_hv y-momentum
     */
    void write(
        tsunami_lab::t_real i_simTime,
        const std::vector<tsunami_lab::t_real> &i_h,
        const std::vector<tsunami_lab::t_real> &i_hu,
        const std::vector<tsunami_lab::t_real> &i_hv
    );
};

} // namespace io
} // namespace tsunami_lab

#endif