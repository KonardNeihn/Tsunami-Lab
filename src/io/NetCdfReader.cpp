#include "NetCdfReader.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

// Helper to check and display errors
void tsunami_lab::io::NetCdfReader::checkNcErr( int         i_status,
                                           const char *i_context ) const {
  if( i_status != NC_NOERR ) {
    std::cerr << "NetCdf error in '" << i_context << "': "
              << nc_strerror( i_status ) << std::endl;
    std::abort();
  }
}

tsunami_lab::io::NetCdfReader::NetCdfReader() {}
tsunami_lab::io::NetCdfReader::~NetCdfReader() { }

std::vector<tsunami_lab::t_real> tsunami_lab::io::NetCdfReader::read(const std::string &path, const std::string &var) {
    int ncid, varid;
    checkNcErr(nc_open(path.c_str(), NC_NOWRITE, &ncid), "nc_open");
    checkNcErr(nc_inq_varid(ncid, var.c_str(), &varid), "nc_inq_varid");

    nc_type type;
    int ndims;
    int dimIds[NC_MAX_DIMS];

    checkNcErr(nc_inq_var(ncid, varid, nullptr, &type, &ndims, dimIds, nullptr),
               "nc_inq_var");

    size_t total = 1;

    for (int i = 0; i < ndims; i++) {
        size_t len;
        checkNcErr(nc_inq_dimlen(ncid, dimIds[i], &len), "nc_inq_dimlen");
        total *= len;
    }

    std::vector<tsunami_lab::t_real> data(total);

    if (type == NC_FLOAT) {
        checkNcErr(nc_get_var_float(ncid, varid, data.data()), "nc_get_var_float");
    }
    else if (type == NC_DOUBLE) {
        std::vector<double> tmp(total);
        checkNcErr(nc_get_var_double(ncid, varid, tmp.data()), "nc_get_var_double");

        for (size_t i = 0; i < total; i++)
            data[i] = static_cast<tsunami_lab::t_real>(tmp[i]);
    }
    else {
        nc_close(ncid);
        throw std::runtime_error("unsupported netCDF type");
    }

    nc_close(ncid);
    return data;
}