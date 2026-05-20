/**
 * @author 
 *
 * @section DESCRIPTION
 * IO class for writing simulation data to netCDF files following the COARDS convention.
 *
 **/
#ifndef TSUNAMI_LAB_IO_NETCDF_READER_H
#define TSUNAMI_LAB_IO_NETCDF_READER_H

#include <string>
#include <netcdf.h>
#include "../constants.h"
#include <vector>

namespace tsunami_lab {
  namespace io {
    class NetCdfReader;
  }
}

class tsunami_lab::io::NetCdfReader {
  private:
    /**
     * Checks a netCDF return code and aborts with a descriptive message on error.
     *
     * @param i_status  return code from a netCDF call.
     * @param i_context error messages.
     **/
    void checkNcErr( int         i_status,
                     const char *i_context ) const;

  public:
    NetCdfReader();
    ~NetCdfReader();

    // struct to hold a 2D grid read from netCDF, along with its dimensions
    struct NetCdfGrid {
      std::vector<tsunami_lab::t_real> data;
      std::size_t nx;
      std::size_t ny;
    };

    /**
     * Reads a variable from a netCDF file and returns it as a vector.
     *
     * @param i_path      Path to the .nc file to read.
     * @param i_variable  Name of the variable to read.
     * @return            1D Vector containing the variables values. 
     **/
    std::vector<tsunami_lab::t_real> read(const std::string &i_path,
                                                    const std::string &i_variable);

    std::vector<tsunami_lab::t_real> read1D(const std::string &i_path, const std::string &i_variable);
};

#endif 