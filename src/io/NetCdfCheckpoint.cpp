#include "NetCdfCheckpoint.h"

#include <netcdf.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>
#include <cstring>

namespace tsunami_lab::io {

void NetCdfCheckpoint::checkNcErr(int status, const char* context) const {
    if (status != NC_NOERR) {
        std::cerr << "NetCDF error in '" << context << "': "
                  << nc_strerror(status) << std::endl;
        std::abort();
    }
}

NetCdfCheckpoint::NetCdfCheckpoint(Config& config)
: g_config(config) 
{}
NetCdfCheckpoint::~NetCdfCheckpoint() {}

void NetCdfCheckpoint::readCheckpointAndSetParameters(
    std::string checkpointPath)
{
    int ncid;
    checkNcErr(nc_open(checkpointPath.c_str(), NC_NOWRITE, &ncid), "nc_open");

    int temp_nx = 0;
    int temp_ny = 0;
    checkNcErr(nc_get_att_int(ncid, NC_GLOBAL, "nx", &temp_nx), "nx");
    checkNcErr(nc_get_att_int(ncid, NC_GLOBAL, "ny", &temp_ny), "ny");
    g_config.nx = temp_nx;
    g_config.ny = temp_ny;

    checkNcErr(nc_get_att_float(ncid, NC_GLOBAL, "lastTimeStep", &g_config.simTime), "lastTimeStep");
    checkNcErr(nc_get_att_float(ncid, NC_GLOBAL, "endTime", &g_config.endTime), "endTime");

    checkNcErr(nc_get_att_float(ncid, NC_GLOBAL, "cellSize", &g_config.dxy), "cellSize");

    checkNcErr(nc_get_att_float(ncid, NC_GLOBAL, "width", &g_config.width), "width");
    
    checkNcErr(nc_get_att_float(ncid, NC_GLOBAL, "domainStartX", &g_config.domainStartX), "domainStartX");
    checkNcErr(nc_get_att_float(ncid, NC_GLOBAL, "domainStartY", &g_config.domainStartY), "domainStartY");

    checkNcErr(nc_close(ncid), "nc_close");
}

std::vector<std::vector<t_real>> NetCdfCheckpoint::read2DVariable(
    std::string checkpointPath,
    std::string varName)
{
    int ncid;
    checkNcErr(nc_open(checkpointPath.c_str(), NC_NOWRITE, &ncid), "nc_open");

    int varid;
    checkNcErr(nc_inq_varid(ncid, varName.c_str(), &varid), "inq_varid");

    int ndims;
    checkNcErr(nc_inq_varndims(ncid, varid, &ndims), "inq_varndims");

    std::vector<int> dimids(ndims);
    checkNcErr(nc_inq_vardimid(ncid, varid, dimids.data()), "inq_vardimid");

    size_t ny, nx;
    checkNcErr(nc_inq_dimlen(ncid, dimids[0], &ny), "dim0");
    checkNcErr(nc_inq_dimlen(ncid, dimids[1], &nx), "dim1");

    std::vector<t_real> flat(nx * ny);

    checkNcErr(
        nc_get_var_float(ncid, varid, flat.data()),
        "read_var"
    );

    checkNcErr(nc_close(ncid), "nc_close");

    std::vector<std::vector<t_real>> result(ny, std::vector<t_real>(nx));

    for (size_t y = 0; y < ny; y++) {
        for (size_t x = 0; x < nx; x++) {
            result[y][x] = flat[y * nx + x];
        }
    }

    return result;
}

void NetCdfCheckpoint::createCheckpoint(
    std::string checkpointPath,
    t_real i_simTime)
{
    int ncid;
    checkNcErr(nc_create(checkpointPath.c_str(), NC_CLOBBER, &ncid), "nc_create");

    // --- 1. DEFINE MODE: Create dimensions and variables ---
    int dim_x, dim_y;
    checkNcErr(nc_def_dim(ncid, "x", g_config.nx, &dim_x), "dim_x");
    checkNcErr(nc_def_dim(ncid, "y", g_config.ny, &dim_y), "dim_y");

    int dims[2] = {dim_y, dim_x};

    int var_height, var_bathy, var_mx, var_my;
    checkNcErr(nc_def_var(ncid, "height", NC_FLOAT, 2, dims, &var_height), "height");
    checkNcErr(nc_def_var(ncid, "bathymetry", NC_FLOAT, 2, dims, &var_bathy), "bathymetry");
    checkNcErr(nc_def_var(ncid, "momentumX", NC_FLOAT, 2, dims, &var_mx), "momentumX");
    checkNcErr(nc_def_var(ncid, "momentumY", NC_FLOAT, 2, dims, &var_my), "momentumY");

    int dimX_var, dimY_var;
    checkNcErr(nc_def_var(ncid, "coordX", NC_FLOAT, 1, &dim_x, &dimX_var), "coordX");
    checkNcErr(nc_def_var(ncid, "coordY", NC_FLOAT, 1, &dim_y, &dimY_var), "coordY");

    // --- 2. WRITE ATTRIBUTES (Still in Define Mode) ---
    int temp_i_nx = static_cast<int>(g_config.nx);
    int temp_i_ny = static_cast<int>(g_config.ny);
    checkNcErr(nc_put_att_int(ncid, NC_GLOBAL, "nx", NC_INT, 1, &temp_i_nx), "nx");
    checkNcErr(nc_put_att_int(ncid, NC_GLOBAL, "ny", NC_INT, 1, &temp_i_ny), "ny");

    checkNcErr(nc_put_att_float(ncid, NC_GLOBAL, "lastTimeStep", NC_FLOAT, 1, &i_simTime), "lastTime");
    checkNcErr(nc_put_att_float(ncid, NC_GLOBAL, "endTime", NC_FLOAT, 1, &g_config.endTime), "endTime");

    checkNcErr(nc_put_att_float(ncid, NC_GLOBAL, "cellSize", NC_FLOAT, 1, &g_config.dxy), "cellSize");

    nc_put_att_float(ncid, NC_GLOBAL, "width", NC_FLOAT, 1, &g_config.width);

    checkNcErr(nc_put_att_float(ncid, NC_GLOBAL, "domainStartX", NC_FLOAT, 1, &g_config.domainStartX), "dx");
    checkNcErr(nc_put_att_float(ncid, NC_GLOBAL, "domainStartY", NC_FLOAT, 1, &g_config.domainStartY), "dy");

    // --- 3. END DEFINE MODE: Switch to Data Mode ---
    checkNcErr(nc_enddef(ncid), "enddef");

    // --- 4. DATA MODE: Write actual variable arrays ---
    std::vector<t_real> x(g_config.nx), y(g_config.ny);

    for (t_idx ix = 0; ix < g_config.nx; ix++) x[ix] = g_config.domainStartX + ix * g_config.dxy;
    for (t_idx iy = 0; iy < g_config.ny; iy++) y[iy] = g_config.domainStartY + iy * g_config.dxy;

    checkNcErr(nc_put_var_float(ncid, dimX_var, x.data()), "coordX write");
    checkNcErr(nc_put_var_float(ncid, dimY_var, y.data()), "coordY write");

    checkNcErr(nc_close(ncid), "close");
}

void NetCdfCheckpoint::write2DVariable(
    std::string checkpointPath,
    std::string variableName,
    const t_real* data,
    t_idx nx,
    t_idx ny)
{
    int ncid;
    checkNcErr(nc_open(checkpointPath.c_str(), NC_WRITE, &ncid), "open");

    int varid;
    checkNcErr(nc_inq_varid(ncid, variableName.c_str(), &varid), "varid");

    size_t start[2] = {0, 0};
    size_t count[2] = {ny, nx};

    // ghost cells entfernen
    const t_idx stride = nx + 2;
    std::vector<t_real> flat(nx * ny);
    for (t_idx y = 0; y < ny; y++) {
        for (t_idx x = 0; x < nx; x++) {
            flat[y * nx + x] =
                data[(y + 1) * stride + (x + 1)];
        }
    }

    checkNcErr(nc_put_vara_float(ncid, varid, start, count, flat.data()), "write2D");

    checkNcErr(nc_close(ncid), "close");
}

} // namespace tsunami_lab