/**
 * @author 
 *
 * @section DESCRIPTION
 * IO class for writing simulation data to netCDF files following the COARDS convention.
 *
 **/
#ifndef TSUNAMI_LAB_IO_NETCDF_H
#define TSUNAMI_LAB_IO_NETCDF_H

#include <string>
#include <netcdf.h>
#include "../constants.h"
#include <vector>

namespace tsunami_lab {
  namespace io {
    class NetCdf;
  }
}

/**
 * The file stores:
 *  - Coordinate variables: x, y (spatial), time (unlimited dimension)
 *  - Data variables:       bathymetry (b), height (h), momentum_x (hu), momentum_y (hv)
 *
 * Ghost cells are excluded from output by default.
 *
 * The time variable uses the attribute "seconds since the earthquake event" so that
 * ParaView recognises the time dimension and enables animation.
 *
 * Typical usage:
 * @code
 *   tsunami_lab::io::NetCdf l_writer( "output.nc",
 *                                      l_nx, l_ny,
 *                                      l_dxy,
 *                                      l_waveProp->getStride(),
 *                                      l_waveProp->getBathymetry() );
 *   // inside the time loop:
 *   l_writer.write( l_simTime,
 *                   l_waveProp->getHeight(),
 *                   l_waveProp->getMomentumX(),
 *                   l_waveProp->getMomentumY() );
 * @endcode
 **/
class tsunami_lab::io::NetCdf {
  private:
    //! netCDF file id
    int m_ncId = -1;

    //! dimension ids: x, y, time 
    int m_dimXId   = -1;
    int m_dimYId   = -1;
    int m_dimTimeId = -1;

    //! variable ids
    int m_varXId    = -1;
    int m_varYId    = -1;
    int m_varTimeId = -1;
    int m_varBId    = -1;  // bathymetry
    int m_varHId    = -1;  // water height
    int m_varHuId   = -1;  // x-momentum
    int m_varHvId   = -1;  // y-momentum 

    //! number of interior cells in x- and y-directions (ghost cells excluded)
    tsunami_lab::t_idx m_nx = 0;
    tsunami_lab::t_idx m_ny = 0;

    //! stride of the solver arrays (interior + 2 ghost cells in x: m_nx + 2)
    tsunami_lab::t_idx m_stride = 0;

    //! current time step index (used as unlimited dimension record index)
    size_t m_timeStep = 0;

    //! true when the simulation is 2-D (m_ny > 1)
    bool m_is2D = true;

    //! coarsening factor currently still disabled for testing
    //tsunami_lab::t_idx m_k = 1;

    //! number of output cells in x after coarsening: ceil(nx / k)
    //tsunami_lab::t_idx m_nxOut = 0;

    //! number of output cells in y after coarsening: ceil(ny / k)
    //tsunami_lab::t_idx m_nyOut = 0;

    /**
     * Checks a netCDF return code and aborts with a descriptive message on error.
     *
     * @param i_status  return code from a netCDF call.
     * @param i_context error messages.
     **/
    void checkNcErr( int         i_status,
                     const char *i_context ) const;

  public:
    /**
     * Opens (or creates) a netCDF output file and writes the static metadata plus the bathymetry variable.
     *
     * The constructor defines all dimensions (x, y, time), all coordinate and data
     * variables with COARDS-compliant attributes, writes the x and y coordinate
     * arrays, and writes the bathymetry field (which does not change over time).
     *
     * @param i_path        Path to the output .nc file (created or overwritten).
     * @param i_nx          Number of interior cells in x-direction.
     * @param i_ny          Number of interior cells in y-direction (use 1 for 1D).
     * @param i_dxy         Cell size [m] (uniform in x and y).
     * @param i_stride      Stride of the solver arrays (typically i_nx + 2).
     * @param i_b           Pointer to the first *interior* bathymetry value
     *                      (i.e. the pointer returned by WavePropagation::getBathymetry()).
     *                      Ghost cells are NOT written.
     **/
    NetCdf( const std::string          &i_path,
            tsunami_lab::t_idx          i_nx,
            tsunami_lab::t_idx          i_ny,
            tsunami_lab::t_real         i_dxy,
            tsunami_lab::t_idx          i_stride,
            const tsunami_lab::t_real  *i_b //, currently disabled for testing purposes
 //           tsunami_lab::t_idx          i_k = 1 // default 1 = no coarsening
          );  

    // Flushes pending writes and closes the netCDF file.
    ~NetCdf();

    // struct to hold a 2D grid read from netCDF, along with its dimensions
    struct NetCdfGrid {
      std::vector<tsunami_lab::t_real> data;
      std::size_t nx;
      std::size_t ny;
    };

    /**
     * Appends one time record to the netCDF file.
     *
     * Writes the simulation time and the three prognostic fields (h, hu, hv) for
     * all interior cells.  Ghost cells are skipped automatically.
     *
     * @param i_simTime  Physical simulation time [s] to record.
     * @param i_h        Pointer to the first interior water-height value.
     * @param i_hu       Pointer to the first interior x-momentum value.
     * @param i_hv       Pointer to the first interior y-momentum value
     *                  
     **/

    void write( tsunami_lab::t_real        i_simTime,
                const tsunami_lab::t_real *i_h,
                const tsunami_lab::t_real *i_hu,
                const tsunami_lab::t_real *i_hv );
};

#endif 