/**
 * @author 
 *
 * @section DESCRIPTION
 * Implementation of the COARDS-compliant netCDF writer.
 **/
#include "NetCdf.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

// Helper to check and display errors
void tsunami_lab::io::NetCdf::checkNcErr( int         i_status,
                                           const char *i_context ) const {
  if( i_status != NC_NOERR ) {
    std::cerr << "NetCdf error in '" << i_context << "': "
              << nc_strerror( i_status ) << std::endl;
    std::abort();
  }
}

// Constructor – opens the file and writes the static data
tsunami_lab::io::NetCdf::NetCdf( const std::string          &i_path,
                                  tsunami_lab::t_idx          i_nx,
                                  tsunami_lab::t_idx          i_ny,
                                  tsunami_lab::t_real         i_dxy,
                                  tsunami_lab::t_idx          i_stride,
                                  const tsunami_lab::t_real  *i_b )
  : m_nx( i_nx ),
    m_ny( i_ny ),
    m_stride( i_stride ),
    m_is2D( i_ny > 1 )
{
  int l_status;

  // 1. Create the file (overwrite if it already exists (NC_CLOBBER), use classic format)
  l_status = nc_create( i_path.c_str(), NC_CLOBBER | NC_NETCDF4, &m_ncId );
  checkNcErr( l_status, "nc_create" );

  // 2. Global attributes (COARDS requires conventions)
  const char *l_conventions = "COARDS";
  l_status = nc_put_att_text( m_ncId, NC_GLOBAL, "Conventions",
                               std::strlen( l_conventions ), l_conventions );
  checkNcErr( l_status, "put_att Conventions" );

  const char *l_title = "Tsunami-Lab Water Simulation";
  l_status = nc_put_att_text( m_ncId, NC_GLOBAL, "title",
                               std::strlen( l_title ), l_title );
  checkNcErr( l_status, "put_att title" );

  // 3. Define dimensions
  //    COARDS order for 2D+time data: (time, y, x)
  // time: unlimited so records can be appended without knowing the count beforehand
  l_status = nc_def_dim( m_ncId, "time", NC_UNLIMITED, &m_dimTimeId );
  checkNcErr( l_status, "def_dim time" );

  l_status = nc_def_dim( m_ncId, "x", static_cast<size_t>( i_nx ), &m_dimXId );
  checkNcErr( l_status, "def_dim x" );

  if( m_is2D ) {
    l_status = nc_def_dim( m_ncId, "y", static_cast<size_t>( i_ny ), &m_dimYId );
    checkNcErr( l_status, "def_dim y" );
  }

  // 4. Define coordinate variables
  // --- time ---
  {
    int l_dimIds[1] = { m_dimTimeId };
    l_status = nc_def_var( m_ncId, "time", NC_FLOAT, 1, l_dimIds, &m_varTimeId );
    checkNcErr( l_status, "def_var time" );

    // "seconds since the earthquake event" triggers the time domain in ParaView
    const char *l_timeUnits = "seconds since the earthquake event";
    l_status = nc_put_att_text( m_ncId, m_varTimeId, "units",
                                 std::strlen( l_timeUnits ), l_timeUnits );
    checkNcErr( l_status, "put_att time units" );

    const char *l_timeStdName = "time";
    l_status = nc_put_att_text( m_ncId, m_varTimeId, "standard_name",
                                 std::strlen( l_timeStdName ), l_timeStdName );
    checkNcErr( l_status, "put_att time standard_name" );
  }

  // --- x coordinate ---
  {
    int l_dimIds[1] = { m_dimXId };
    l_status = nc_def_var( m_ncId, "x", NC_FLOAT, 1, l_dimIds, &m_varXId );
    checkNcErr( l_status, "def_var x" );

    const char *l_xUnits = "meters";
    l_status = nc_put_att_text( m_ncId, m_varXId, "units",
                                 std::strlen( l_xUnits ), l_xUnits );
    checkNcErr( l_status, "put_att x units" );

    const char *l_xLongName = "x-coordinate of cell centers";
    l_status = nc_put_att_text( m_ncId, m_varXId, "long_name",
                                 std::strlen( l_xLongName ), l_xLongName );
    checkNcErr( l_status, "put_att x long_name" );

    const char *l_xAxis = "X";
    l_status = nc_put_att_text( m_ncId, m_varXId, "axis",
                                 std::strlen( l_xAxis ), l_xAxis );
    checkNcErr( l_status, "put_att x axis" );
  }

  // --- y coordinate ---
  if( m_is2D ) {
    int l_dimIds[1] = { m_dimYId };
    l_status = nc_def_var( m_ncId, "y", NC_FLOAT, 1, l_dimIds, &m_varYId );
    checkNcErr( l_status, "def_var y" );

    const char *l_yUnits = "meters";
    l_status = nc_put_att_text( m_ncId, m_varYId, "units",
                                 std::strlen( l_yUnits ), l_yUnits );
    checkNcErr( l_status, "put_att y units" );

    const char *l_yLongName = "y-coordinate of cell centers";
    l_status = nc_put_att_text( m_ncId, m_varYId, "long_name",
                                 std::strlen( l_yLongName ), l_yLongName );
    checkNcErr( l_status, "put_att y long_name" );

    const char *l_yAxis = "Y";
    l_status = nc_put_att_text( m_ncId, m_varYId, "axis",
                                 std::strlen( l_yAxis ), l_yAxis );
    checkNcErr( l_status, "put_att y axis" );
  }

  // 5. Define data variables
  //    Layout: (time, y, x)

  // Helper to build dimension id array depending on dimensionality and return the number of dimensions
  auto buildDimIds = [&]( int o_dimIds[3] ) -> int {
    if( m_is2D ) {
      o_dimIds[0] = m_dimTimeId;
      o_dimIds[1] = m_dimYId;
      o_dimIds[2] = m_dimXId;
      return 3;
    } else {
      o_dimIds[0] = m_dimTimeId;
      o_dimIds[1] = m_dimXId;
      return 2;
    }
  };

  int l_dataDimIds[3];
  int l_nDims = buildDimIds( l_dataDimIds );

  // bathymetry b (time-independent – stored as (y,x) or (x) without time)
  {
    int l_bDimIds[2];
    int l_bNDims;
    if( m_is2D ) {
      l_bDimIds[0] = m_dimYId;
      l_bDimIds[1] = m_dimXId;
      l_bNDims = 2;
    } else {
      l_bDimIds[0] = m_dimXId;
      l_bNDims = 1;
    }
    l_status = nc_def_var( m_ncId, "b", NC_FLOAT, l_bNDims, l_bDimIds, &m_varBId );
    checkNcErr( l_status, "def_var b" );

    const char *l_bLongName = "bathymetry (seafloor elevation)";
    l_status = nc_put_att_text( m_ncId, m_varBId, "long_name",
                                 std::strlen( l_bLongName ), l_bLongName );
    checkNcErr( l_status, "put_att b long_name" );

    const char *l_bUnits = "meters";
    l_status = nc_put_att_text( m_ncId, m_varBId, "units",
                                 std::strlen( l_bUnits ), l_bUnits );
    checkNcErr( l_status, "put_att b units" );
  }

  // water height h
  {
    l_status = nc_def_var( m_ncId, "h", NC_FLOAT, l_nDims, l_dataDimIds, &m_varHId );
    checkNcErr( l_status, "def_var h" );

    const char *l_hLongName = "water height above bathymetry";
    l_status = nc_put_att_text( m_ncId, m_varHId, "long_name",
                                 std::strlen( l_hLongName ), l_hLongName );
    checkNcErr( l_status, "put_att h long_name" );

    const char *l_hUnits = "meters";
    l_status = nc_put_att_text( m_ncId, m_varHId, "units",
                                 std::strlen( l_hUnits ), l_hUnits );
    checkNcErr( l_status, "put_att h units" );
  }

  // x-momentum hu
  {
    l_status = nc_def_var( m_ncId, "hu", NC_FLOAT, l_nDims, l_dataDimIds, &m_varHuId );
    checkNcErr( l_status, "def_var hu" );

    const char *l_huLongName = "x-momentum (h * u)";
    l_status = nc_put_att_text( m_ncId, m_varHuId, "long_name",
                                 std::strlen( l_huLongName ), l_huLongName );
    checkNcErr( l_status, "put_att hu long_name" );

    const char *l_huUnits = "meters^2 / second";
    l_status = nc_put_att_text( m_ncId, m_varHuId, "units",
                                 std::strlen( l_huUnits ), l_huUnits );
    checkNcErr( l_status, "put_att hu units" );
  }

  // y-momentum hv (2D only)
  if( m_is2D ) {
    l_status = nc_def_var( m_ncId, "hv", NC_FLOAT, l_nDims, l_dataDimIds, &m_varHvId );
    checkNcErr( l_status, "def_var hv" );

    const char *l_hvLongName = "y-momentum (h * v)";
    l_status = nc_put_att_text( m_ncId, m_varHvId, "long_name",
                                 std::strlen( l_hvLongName ), l_hvLongName );
    checkNcErr( l_status, "put_att hv long_name" );

    const char *l_hvUnits = "meters^2 / second";
    l_status = nc_put_att_text( m_ncId, m_varHvId, "units",
                                 std::strlen( l_hvUnits ), l_hvUnits );
    checkNcErr( l_status, "put_att hv units" );
  }

  // 6. Leave define mode
  l_status = nc_enddef( m_ncId );
  checkNcErr( l_status, "nc_enddef" );

  // 7. Write coordinate arrays (x, y)
  // x: cell centers at (ix + 0.5) * dxy  for ix in [0, nx)
  {
    std::vector<float> l_xCoords( i_nx );
    for( tsunami_lab::t_idx l_ix = 0; l_ix < i_nx; ++l_ix )
      l_xCoords[l_ix] = static_cast<float>( (l_ix + 0.5) * i_dxy );

    l_status = nc_put_var_float( m_ncId, m_varXId, l_xCoords.data() );
    checkNcErr( l_status, "put_var x" );
  }

  if( m_is2D ) {
    std::vector<float> l_yCoords( i_ny );
    for( tsunami_lab::t_idx l_iy = 0; l_iy < i_ny; ++l_iy )
      l_yCoords[l_iy] = static_cast<float>( (l_iy + 0.5) * i_dxy );

    l_status = nc_put_var_float( m_ncId, m_varYId, l_yCoords.data() );
    checkNcErr( l_status, "put_var y" );
  }

  // 8. Write bathymetry (static – does not change over time)
  //    The solver arrays are padded with ghost cells; we copy only interior cells.
  {
    // Interior cells: ix in [0, nx), iy in [0, ny)
    // i_b points at the first interior cell of the solver array, with stride m_stride.
    // For 1D (m_ny == 1) the inner loop runs once.
    const size_t l_totalInterior = static_cast<size_t>( i_nx ) *
                                   static_cast<size_t>( m_is2D ? i_ny : 1 );
    std::vector<float> l_buf( l_totalInterior );

    if( m_is2D ) {
      for( tsunami_lab::t_idx l_iy = 0; l_iy < i_ny; ++l_iy )
        for( tsunami_lab::t_idx l_ix = 0; l_ix < i_nx; ++l_ix )
          l_buf[ l_iy * i_nx + l_ix ] =
              static_cast<float>( i_b[ l_iy * m_stride + l_ix ] );
    } else {
      for( tsunami_lab::t_idx l_ix = 0; l_ix < i_nx; ++l_ix )
        l_buf[l_ix] = static_cast<float>( i_b[l_ix] );
    }

    l_status = nc_put_var_float( m_ncId, m_varBId, l_buf.data() );
    checkNcErr( l_status, "put_var b" );
  }
}

// Destructor
tsunami_lab::io::NetCdf::~NetCdf() {
  if( m_ncId >= 0 ) {
    nc_close( m_ncId );
    m_ncId = -1;
  }
}

// write() – appends one time record
void tsunami_lab::io::NetCdf::write( tsunami_lab::t_real        i_simTime,
                                      const tsunami_lab::t_real *i_h,
                                      const tsunami_lab::t_real *i_hu,
                                      const tsunami_lab::t_real *i_hv ) {
  int l_status;

  // 1. Append the simulation time to the unlimited time variable
  {
    size_t l_start[1] = { m_timeStep };
    size_t l_count[1] = { 1 };
    float  l_t        = static_cast<float>( i_simTime );

    l_status = nc_put_vara_float( m_ncId, m_varTimeId, l_start, l_count, &l_t );
    checkNcErr( l_status, "put_vara time" );
  }

  // 2. Pack interior cells into a contiguous buffer (without ghost cells)
  const size_t l_nx  = static_cast<size_t>( m_nx );
  const size_t l_ny  = static_cast<size_t>( m_is2D ? m_ny : 1 );
  const size_t l_total = l_nx * l_ny;

  std::vector<float> l_hBuf ( l_total );
  std::vector<float> l_huBuf( l_total );
  std::vector<float> l_hvBuf;
  if( m_is2D && i_hv != nullptr )
    l_hvBuf.resize( l_total );

  if( m_is2D ) {
    for( size_t l_iy = 0; l_iy < l_ny; ++l_iy ) {
      for( size_t l_ix = 0; l_ix < l_nx; ++l_ix ) {
        const size_t l_solverIdx = l_iy * m_stride + l_ix;
        const size_t l_bufIdx    = l_iy * l_nx + l_ix;

        l_hBuf [l_bufIdx] = static_cast<float>( i_h [ l_solverIdx ] );
        l_huBuf[l_bufIdx] = static_cast<float>( i_hu[ l_solverIdx ] );
        if( !l_hvBuf.empty() )
          l_hvBuf[l_bufIdx] = static_cast<float>( i_hv[ l_solverIdx ] );
      }
    }
  } else {
    // 1D: single row, stride is irrelevant for the copy
    for( size_t l_ix = 0; l_ix < l_nx; ++l_ix ) {
      l_hBuf [l_ix] = static_cast<float>( i_h [ l_ix ] );
      l_huBuf[l_ix] = static_cast<float>( i_hu[ l_ix ] );
    }
  }

  // 3. Write the current record into the netCDF file
  //    For 2D: start = {timeStep, 0, 0}, count = {1, ny, nx}
  //    For 1D: start = {timeStep, 0},    count = {1, nx}
  if( m_is2D ) {
    size_t l_start[3] = { m_timeStep, 0, 0 };
    size_t l_count[3] = { 1, l_ny, l_nx };

    l_status = nc_put_vara_float( m_ncId, m_varHId,  l_start, l_count, l_hBuf.data() );
    checkNcErr( l_status, "put_vara h (2D)" );

    l_status = nc_put_vara_float( m_ncId, m_varHuId, l_start, l_count, l_huBuf.data() );
    checkNcErr( l_status, "put_vara hu (2D)" );

    if( !l_hvBuf.empty() ) {
      l_status = nc_put_vara_float( m_ncId, m_varHvId, l_start, l_count, l_hvBuf.data() );
      checkNcErr( l_status, "put_vara hv (2D)" );
    }
  } else {
    size_t l_start[2] = { m_timeStep, 0 };
    size_t l_count[2] = { 1, l_nx };

    l_status = nc_put_vara_float( m_ncId, m_varHId,  l_start, l_count, l_hBuf.data() );
    checkNcErr( l_status, "put_vara h (1D)" );

    l_status = nc_put_vara_float( m_ncId, m_varHuId, l_start, l_count, l_huBuf.data() );
    checkNcErr( l_status, "put_vara hu (1D)" );
  }

  // 4. Advance the record index
  ++m_timeStep;
}