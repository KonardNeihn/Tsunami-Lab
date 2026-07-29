/**
 * @section DESCRIPTION
 * Unit tests for TsunamiEvent2d setup.
 **/
#include <catch2/catch.hpp>
#include <filesystem>
#include <vector>
#include <netcdf.h>

#include "TsunamiEvent2d.h"
#include "../config/Config.h"

// create Dummy for 2D-NetCDF Datei (x, y, z) for Bathymetry andDisplacement
static void createDummyNetCdf(
    const std::string& path,
    const std::vector<float>& x,
    const std::vector<float>& y,
    const std::vector<float>& z) 
{
    int ncid, dimX, dimY, varX, varY, varZ;
    
    if (nc_create(path.c_str(), NC_CLOBBER, &ncid) != NC_NOERR) return;

    nc_def_dim(ncid, "x", x.size(), &dimX);
    nc_def_dim(ncid, "y", y.size(), &dimY);

    nc_def_var(ncid, "x", NC_FLOAT, 1, &dimX, &varX);
    nc_def_var(ncid, "y", NC_FLOAT, 1, &dimY, &varY);

    int dims2D[2] = {dimY, dimX}; // Row-Major: [y, x]
    nc_def_var(ncid, "z", NC_FLOAT, 2, dims2D, &varZ);

    nc_enddef(ncid);

    nc_put_var_float(ncid, varX, x.data());
    nc_put_var_float(ncid, varY, y.data());
    nc_put_var_float(ncid, varZ, z.data());

    nc_close(ncid);
}

TEST_CASE("TsunamiEvent2d: Domain setup and Data lookup", "[TsunamiEvent2d]") {
    std::string l_bathFile = "solutions/test_bathymetry.nc";
    std::string l_dispFile = "solutions/test_displacement.nc";

    if (std::filesystem::exists(l_bathFile)) std::filesystem::remove(l_bathFile);
    if (std::filesystem::exists(l_dispFile)) std::filesystem::remove(l_dispFile);

    // X ynd Y start at 0.0, so domainStartX/Y = 0.0 
    std::vector<float> x = {0.0f, 100.0f};
    std::vector<float> y = {0.0f, 100.0f};

    // Bathymetry: [y][x]
    // Row 0 (y=0):   -100.0, -100.0
    // Row 1 (y=100): -100.0,  -50.0
    std::vector<float> b_z = {
        -100.0f, -100.0f,
        -100.0f,  -50.0f
    };

    // Displacement
    std::vector<float> d_z = {
        5.0f, 0.0f,
        0.0f, 0.0f
    };

    createDummyNetCdf(l_bathFile, x, y, b_z);
    createDummyNetCdf(l_dispFile, x, y, d_z);

    Config l_config;
    l_config.nx = 2; // Domain [0, 100] divided into two rwos (dxy = 50)

    tsunami_lab::setups::TsunamiEvent2d l_tsunami(l_config, l_bathFile, l_dispFile);

    SECTION("Config and Domain calculations") {
        REQUIRE(l_config.is_2d == true);
        REQUIRE(l_config.domainStartX == Approx(0.0));
        REQUIRE(l_config.domainStartY == Approx(0.0));
        REQUIRE(l_config.width == Approx(100.0));
        REQUIRE(l_config.dxy == Approx(50.0));
        REQUIRE(l_config.ny == 2);
    }

    SECTION("Bathymetry lookup") {
        // kurze Erklärung, weil ich es auch erst nicht vestanden hatte:
        // Zelle (0,0) ist physikalisch (25, 25).
        // lower_bound findet in {0, 100} das Element 100 (Index 1).
        // Wenn Code dann nach [1][1] greift, erhält er -50.0. 
        // da dann [0][0] erwartet wird, testen wir hier Zelle (0,0):
        REQUIRE(l_tsunami.getBathymetry(0, 0) == Approx(-50.0)); 
    }

    SECTION("Height calculation") {
        // Index (1,1) -> b = -50.0, d = 0.0 -> h = 50.0
        REQUIRE(l_tsunami.getHeight(0, 0) == Approx(50.0));
    }

    // Cleanup
    if (std::filesystem::exists(l_bathFile)) std::filesystem::remove(l_bathFile);
    if (std::filesystem::exists(l_dispFile)) std::filesystem::remove(l_dispFile);
}