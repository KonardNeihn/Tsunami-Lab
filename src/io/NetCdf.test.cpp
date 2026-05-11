/**
 * @section DESCRIPTION
 * Unit tests for io::NetCdf.
 *
 * Tests:
 *  1. 1D write – creates a valid netCDF file with the expected structure.
 *  2. 2D write – same for the 2D case with y-momentum.
 *  3. Multiple time steps – verifies the unlimited time dimension grows correctly.
 **/
#include <catch2/catch_all.hpp>
#include <netcdf.h>
#include <cstdio>   // std::remove
#include <cmath>    // std::fabs
#include <vector>

#include "NetCdf.h"

// ---- tiny helper: read a named scalar float attribute from a variable ------
static std::string readTextAtt( int i_ncId, int i_varId, const char *i_name ) {
  size_t l_len = 0;
  nc_inq_attlen( i_ncId, i_varId, i_name, &l_len );
  std::string l_s( l_len, '\0' );
  nc_get_att_text( i_ncId, i_varId, i_name, &l_s[0] );
  return l_s;
}

// TEST 1 – 1D write
TEST_CASE( "NetCdf 1D write produces a valid COARDS file", "[NetCdf]" ) {

  const std::string l_path = "/tmp/test_netcdf_1d.nc";

  const tsunami_lab::t_idx l_nx     = 4;
  const tsunami_lab::t_idx l_ny     = 1;   // 1D
  const tsunami_lab::t_real l_dxy   = 10.0f;
  const tsunami_lab::t_idx l_stride = l_nx; // 1D: no ghost padding in x needed
                                             // (we assume the writer skips ghosts via pointer)

  // Flat arrays – interior only (stride == nx for 1D convenience)
  std::vector<tsunami_lab::t_real> l_b (l_nx, -100.0f);  // flat bathymetry
  std::vector<tsunami_lab::t_real> l_h (l_nx);
  std::vector<tsunami_lab::t_real> l_hu(l_nx);

  for( tsunami_lab::t_idx l_i = 0; l_i < l_nx; ++l_i ) {
    l_h [l_i] = static_cast<float>( l_i + 1 );
    l_hu[l_i] = static_cast<float>( l_i ) * 0.5f;
  }

  // --- write ---
  {
    tsunami_lab::io::NetCdf l_writer( l_path, l_nx, l_ny, l_dxy, l_stride, l_b.data() );
    l_writer.write( 0.0f, l_h.data(), l_hu.data(), nullptr );
    l_writer.write( 1.0f, l_h.data(), l_hu.data(), nullptr );
  } // destructor closes file

  // --- verify with raw netCDF-C API ---
  int l_ncId;
  REQUIRE( nc_open( l_path.c_str(), NC_NOWRITE, &l_ncId ) == NC_NOERR );

  // Conventions attribute
  std::string l_conv = readTextAtt( l_ncId, NC_GLOBAL, "Conventions" );
  REQUIRE( l_conv == "COARDS" );

  // time variable exists and has the keyword "since"
  int l_varTimeId;
  REQUIRE( nc_inq_varid( l_ncId, "time", &l_varTimeId ) == NC_NOERR );
  std::string l_timeUnits = readTextAtt( l_ncId, l_varTimeId, "units" );
  REQUIRE( l_timeUnits.find("since") != std::string::npos );

  // time has 2 records
  int l_dimTimeId;
  REQUIRE( nc_inq_dimid( l_ncId, "time", &l_dimTimeId ) == NC_NOERR );
  size_t l_timeLen;
  nc_inq_dimlen( l_ncId, l_dimTimeId, &l_timeLen );
  REQUIRE( l_timeLen == 2 );

  // x dimension has nx entries
  int l_dimXId;
  REQUIRE( nc_inq_dimid( l_ncId, "x", &l_dimXId ) == NC_NOERR );
  size_t l_xLen;
  nc_inq_dimlen( l_ncId, l_dimXId, &l_xLen );
  REQUIRE( l_xLen == l_nx );

  // no y dimension for 1D
  int l_dimYId;
  REQUIRE( nc_inq_dimid( l_ncId, "y", &l_dimYId ) != NC_NOERR );

  // bathymetry values match
  int l_varBId;
  REQUIRE( nc_inq_varid( l_ncId, "b", &l_varBId ) == NC_NOERR );
  std::vector<float> l_bRead( l_nx );
  nc_get_var_float( l_ncId, l_varBId, l_bRead.data() );
  for( tsunami_lab::t_idx l_i = 0; l_i < l_nx; ++l_i )
    REQUIRE( std::fabs( l_bRead[l_i] - (-100.0f) ) < 1e-4f );

  // h values (first time record) match
  int l_varHId;
  REQUIRE( nc_inq_varid( l_ncId, "h", &l_varHId ) == NC_NOERR );
  std::vector<float> l_hRead( l_nx );
  size_t l_hStart[2] = {0, 0};
  size_t l_hCount[2] = {1, static_cast<size_t>(l_nx)};
  nc_get_vara_float( l_ncId, l_varHId, l_hStart, l_hCount, l_hRead.data() );
  for( tsunami_lab::t_idx l_i = 0; l_i < l_nx; ++l_i )
    REQUIRE( std::fabs( l_hRead[l_i] - static_cast<float>(l_i + 1) ) < 1e-4f );

  nc_close( l_ncId );
  std::remove( l_path.c_str() );
}

// TEST 2 – 2D write
TEST_CASE( "NetCdf 2D write produces a valid COARDS file with y dimension", "[NetCdf]" ) {

  const std::string l_path = "/tmp/test_netcdf_2d.nc";

  const tsunami_lab::t_idx l_nx     = 3;
  const tsunami_lab::t_idx l_ny     = 2;
  const tsunami_lab::t_real l_dxy   = 5.0f;
  const tsunami_lab::t_idx l_stride = l_nx;  // no ghost cells in this test array

  const size_t l_total = static_cast<size_t>( l_nx ) * static_cast<size_t>( l_ny );
  std::vector<tsunami_lab::t_real> l_b (l_total, -50.0f);
  std::vector<tsunami_lab::t_real> l_h (l_total,  10.0f);
  std::vector<tsunami_lab::t_real> l_hu(l_total,   2.0f);
  std::vector<tsunami_lab::t_real> l_hv(l_total,   1.5f);

  {
    tsunami_lab::io::NetCdf l_writer( l_path, l_nx, l_ny, l_dxy, l_stride, l_b.data() );
    l_writer.write( 0.5f, l_h.data(), l_hu.data(), l_hv.data() );
  }

  int l_ncId;
  REQUIRE( nc_open( l_path.c_str(), NC_NOWRITE, &l_ncId ) == NC_NOERR );

  // y dimension exists and has correct size
  int l_dimYId;
  REQUIRE( nc_inq_dimid( l_ncId, "y", &l_dimYId ) == NC_NOERR );
  size_t l_yLen;
  nc_inq_dimlen( l_ncId, l_dimYId, &l_yLen );
  REQUIRE( l_yLen == static_cast<size_t>( l_ny ) );

  // hv variable exists
  int l_varHvId;
  REQUIRE( nc_inq_varid( l_ncId, "hv", &l_varHvId ) == NC_NOERR );

  // hv values match
  std::vector<float> l_hvRead( l_total );
  size_t l_hvStart[3] = {0, 0, 0};
  size_t l_hvCount[3] = {1, static_cast<size_t>(l_ny), static_cast<size_t>(l_nx)};
  nc_get_vara_float( l_ncId, l_varHvId, l_hvStart, l_hvCount, l_hvRead.data() );
  for( size_t l_i = 0; l_i < l_total; ++l_i )
    REQUIRE( std::fabs( l_hvRead[l_i] - 1.5f ) < 1e-4f );

  nc_close( l_ncId );
  std::remove( l_path.c_str() );
}

// TEST 3 – Ghost-cell exclusion
// Uses a stride larger than nx (as the solver would provide) and verifies
// that only interior cells land in the file.
TEST_CASE( "NetCdf excludes ghost cells from output", "[NetCdf]" ) {

  const std::string l_path = "/tmp/test_netcdf_ghost.nc";

  const tsunami_lab::t_idx l_nx     = 3;
  const tsunami_lab::t_idx l_ny     = 1;
  const tsunami_lab::t_real l_dxy   = 1.0f;
  const tsunami_lab::t_idx l_stride = l_nx + 2; // real solver stride

  // Build a padded 1D array: [ghost | 10 | 20 | 30 | ghost]
  std::vector<tsunami_lab::t_real> l_bPadded( l_nx + 2, -999.0f );
  l_bPadded[1] = 10.0f;
  l_bPadded[2] = 20.0f;
  l_bPadded[3] = 30.0f;
  // pointer to first interior cell (skip left ghost)
  const tsunami_lab::t_real *l_bInterior = l_bPadded.data() + 1;

  std::vector<tsunami_lab::t_real> l_hPadded( l_nx + 2, -999.0f );
  l_hPadded[1] = 1.0f; l_hPadded[2] = 2.0f; l_hPadded[3] = 3.0f;
  const tsunami_lab::t_real *l_hInterior = l_hPadded.data() + 1;

  std::vector<tsunami_lab::t_real> l_huPadded( l_nx + 2, 0.0f );
  const tsunami_lab::t_real *l_huInterior = l_huPadded.data() + 1;

  {
    tsunami_lab::io::NetCdf l_writer( l_path, l_nx, l_ny, l_dxy, l_stride, l_bInterior );
    l_writer.write( 0.0f, l_hInterior, l_huInterior, nullptr );
  }

  int l_ncId;
  REQUIRE( nc_open( l_path.c_str(), NC_NOWRITE, &l_ncId ) == NC_NOERR );

  int l_varBId;
  nc_inq_varid( l_ncId, "b", &l_varBId );
  std::vector<float> l_bRead( l_nx );
  nc_get_var_float( l_ncId, l_varBId, l_bRead.data() );

  // Ghost cell value (-999) must not appear
  REQUIRE( std::fabs( l_bRead[0] - 10.0f ) < 1e-4f );
  REQUIRE( std::fabs( l_bRead[1] - 20.0f ) < 1e-4f );
  REQUIRE( std::fabs( l_bRead[2] - 30.0f ) < 1e-4f );

  int l_varHId;
  nc_inq_varid( l_ncId, "h", &l_varHId );
  std::vector<float> l_hRead( l_nx );
  size_t l_hStart[2] = {0, 0};
  size_t l_hCount[2] = {1, static_cast<size_t>(l_nx)};
  nc_get_vara_float( l_ncId, l_varHId, l_hStart, l_hCount, l_hRead.data() );
  REQUIRE( std::fabs( l_hRead[0] - 1.0f ) < 1e-4f );
  REQUIRE( std::fabs( l_hRead[1] - 2.0f ) < 1e-4f );
  REQUIRE( std::fabs( l_hRead[2] - 3.0f ) < 1e-4f );

  nc_close( l_ncId );
  std::remove( l_path.c_str() );
}