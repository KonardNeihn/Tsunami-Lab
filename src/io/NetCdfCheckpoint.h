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

  public:
    NetCdfCheckpoint();
    ~NetCdfCheckpoint();

    /*
    *
    */
    void readCheckpointAndSetParameters(std::string checkpointPath,
                                  t_idx &o_nx,
                                  t_idx &o_ny,
                                  t_real &o_lastTimeStep,
                                  t_real &o_endTime,
                                  t_real &o_w,
                                  t_real &o_domainStartX,
                                  t_real &o_domainStartY);

    /*
    *
    */
    std::vector<std::vector<t_real>> read2DVariable(std::string checkpointPath, std::string varName);

    void createCheckpoint(std::string checkpointPath,
                         t_idx i_nx,
                         t_idx i_ny,
                         t_real i_lastTimeStep,
                         t_real i_endTime,
                         t_real i_w,
                         t_real i_domainStartX,
                         t_real i_domainStartY);

    void writeCheckpoint(std::string checkpointPath,
                         std::string variableName,
                        t_real x,
                        t_real y,
                        t_real value);
};

#endif 