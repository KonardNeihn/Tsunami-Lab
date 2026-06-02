/**
 * @author 
 *
 * @section DESCRIPTION
 * write and read checkpoint files in netCDF format.
 *
 **/
#ifndef TSUNAMI_LAB_IO_NETCDF_CHECKPOINT_H
#define TSUNAMI_LAB_IO_NETCDF_CHECKPOINT_H

#include <string>
#include <netcdf.h>
#include "../constants.h"
#include <vector>
#include "../config/Config.h"

namespace tsunami_lab {
  namespace io {
    class NetCdfCheckpoint;
  }
}

class tsunami_lab::io::NetCdfCheckpoint {
  private:
    /**
     * Checks a netCDF return code and aborts with a descriptive message on error.
     *
     * @param i_status  return code from a netCDF call.
     * @param i_context error messages.
     **/
    void checkNcErr( int         i_status,
                     const char *i_context ) const;
    Config& g_config;

  public:
    NetCdfCheckpoint(Config& config);
    ~NetCdfCheckpoint();

    /*
    *
    */
    void readCheckpointAndSetParameters(std::string checkpointPath);

    /*
    *
    */
    std::vector<std::vector<t_real>> read2DVariable(std::string checkpointPath, std::string varName);

    void createCheckpoint(std::string checkpointPath,
                         t_real i_simTime);

    void write2DVariable(std::string checkpointPath,
                                std::string variableName,
                                const t_real* data,
                                t_idx nx,
                                t_idx ny);
};

#endif 