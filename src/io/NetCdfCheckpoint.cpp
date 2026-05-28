#include "NetCdfCheckpoint.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

// Helper to check and display errors
void tsunami_lab::io::NetCdfCheckpoint::checkNcErr( int         i_status,
                                           const char *i_context ) const {
  if( i_status != NC_NOERR ) {
    std::cerr << "NetCdf error in '" << i_context << "': "
              << nc_strerror( i_status ) << std::endl;
    std::abort();
  }
}

tsunami_lab::io::NetCdfCheckpoint::NetCdfCheckpoint() {}
tsunami_lab::io::NetCdfCheckpoint::~NetCdfCheckpoint() {}

void tsunami_lab::io::NetCdfCheckpoint::readCheckpointAndSetParameters(std::string checkpointPath,
                                  t_idx &o_nx,
                                  t_idx &o_ny,
                                  t_real &o_lastTimeStep,
                                  t_real &o_endTime,
                                  t_real &o_w,
                                  t_real &o_domainStartX,
                                  t_real &o_domainStartY) {
    //
}

std::vector<std::vector<tsunami_lab::t_real>> tsunami_lab::io::NetCdfCheckpoint::read2DVariable(std::string checkpointPath, std::string varName) {
    
}

