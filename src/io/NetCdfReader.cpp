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

std::vector<tsunami_lab::t_real>
tsunami_lab::io::NetCdfReader::read(
    const std::string &path,
    const std::string &var)
{
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

std::vector<tsunami_lab::t_real> tsunami_lab::io::NetCdfReader::read1D(const std::string &i_path, const std::string &i_variable)
{
    int l_ncId;
    int l_varId;

    nc_open(i_path.c_str(), NC_NOWRITE, &l_ncId);
    nc_inq_varid(l_ncId, i_variable.c_str(), &l_varId);

    nc_type l_type;
    int l_ndims;
    int l_dimIds[NC_MAX_DIMS];

    nc_inq_var(l_ncId,
               l_varId,
               nullptr,
               &l_type,
               &l_ndims,
               l_dimIds,
               nullptr);

    if( l_ndims != 1 ) {
        throw std::runtime_error("Variable is not 1D");
    }

    size_t l_size;
    nc_inq_dimlen(l_ncId, l_dimIds[0], &l_size);

    std::vector<t_real> l_data(l_size);

    nc_get_var_float(l_ncId, l_varId, l_data.data());

    nc_close(l_ncId);

    return l_data;
}