/**
 * @author Konard Neihn
 *
 * @section DESCRIPTION
 * Implementation of the COARDS-compliant netCDF writer for adaptive grid simulations.
 */
#include "NetCdfAdaptiveGrid.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

// Helper to check and display errors
void tsunami_lab::io::NetCdfAdaptiveGrid::checkNcErr(int i_status, const char* i_context) const {
    if (i_status != NC_NOERR) {
        std::cerr << "NetCDF error in '" << i_context << "': " << nc_strerror(i_status) << std::endl;
        std::abort();
    }
}

// Constructor – opens the file and writes the static data
tsunami_lab::io::NetCdfAdaptiveGrid::NetCdfAdaptiveGrid(
    const std::string &i_path,
    tsunami_lab::t_idx i_nxOut,
    tsunami_lab::t_idx i_nyOut,
    tsunami_lab::t_real i_dxyOut,
    const std::vector<tsunami_lab::t_real> &i_b,
    bool i_isCheckpoint
)
:
m_nxOut(i_nxOut),
m_nyOut(i_nyOut),
m_dxyOut(i_dxyOut),
m_is2D(i_nyOut > 1),
m_isCheckpoint(i_isCheckpoint)
{
    if (m_isCheckpoint) {
        // ggf. später implementieren
        return;
    }

    //----------------------------------------------------------
    // create file
    //----------------------------------------------------------
    checkNcErr(
        nc_create(
            i_path.c_str(),
            NC_NETCDF4 | NC_CLOBBER,
            &m_ncId),
        "nc_create");

    //----------------------------------------------------------
    // global attributes
    //----------------------------------------------------------
    const char* conv = "CF-1.8";

    checkNcErr(
        nc_put_att_text(
            m_ncId,
            NC_GLOBAL,
            "Conventions",
            strlen(conv),
            conv),
        "Conventions");
    
    //----------------------------------------------------------
    // dimensions
    //----------------------------------------------------------
    checkNcErr(
        nc_def_dim(m_ncId, "time", NC_UNLIMITED, &m_dimTimeId),
        "time dimension");

    checkNcErr(
        nc_def_dim(m_ncId, "x", m_nxOut, &m_dimXId),
        "x dimension");

    if (m_is2D) {
        checkNcErr(
            nc_def_dim(m_ncId, "y", m_nyOut, &m_dimYId),
            "y dimension");
    }

    //----------------------------------------------------------
    // coordinate variables
    //----------------------------------------------------------
    checkNcErr(
        nc_def_var(
            m_ncId,
            "time",
            NC_FLOAT,
            1,
            &m_dimTimeId,
            &m_varTimeId),
        "time variable");

    checkNcErr(
        nc_def_var(
            m_ncId,
            "x",
            NC_FLOAT,
            1,
            &m_dimXId,
            &m_varXId),
        "x variable");

    if (m_is2D) {
        checkNcErr(
            nc_def_var(
                m_ncId,
                "y",
                NC_FLOAT,
                1,
                &m_dimYId,
                &m_varYId),
            "y variable");
    }

    //----------------------------------------------------------
    // data variables
    //----------------------------------------------------------
    if (m_is2D) {

        int dims3[3] = {
            m_dimTimeId,
            m_dimYId,
            m_dimXId
        };

        int dims2[2] = {
            m_dimYId,
            m_dimXId
        };

        checkNcErr(
            nc_def_var(
                m_ncId,
                "b",
                NC_FLOAT,
                2,
                dims2,
                &m_varBId),
            "bathymetry");

        checkNcErr(
            nc_def_var(
                m_ncId,
                "h",
                NC_FLOAT,
                3,
                dims3,
                &m_varHId),
            "height");

        checkNcErr(
            nc_def_var(
                m_ncId,
                "hu",
                NC_FLOAT,
                3,
                dims3,
                &m_varHuId),
            "hu");

        checkNcErr(
            nc_def_var(
                m_ncId,
                "hv",
                NC_FLOAT,
                3,
                dims3,
                &m_varHvId),
            "hv");
    }
    else {

        int dims2[2] = {
            m_dimTimeId,
            m_dimXId
        };

        checkNcErr(
            nc_def_var(
                m_ncId,
                "b",
                NC_FLOAT,
                1,
                &m_dimXId,
                &m_varBId),
            "bathymetry");

        checkNcErr(
            nc_def_var(
                m_ncId,
                "h",
                NC_FLOAT,
                2,
                dims2,
                &m_varHId),
            "height");

        checkNcErr(
            nc_def_var(
                m_ncId,
                "hu",
                NC_FLOAT,
                2,
                dims2,
                &m_varHuId),
            "hu");
    }

    //----------------------------------------------------------
    // attributes (noch im define mode!)
    //----------------------------------------------------------

    // time
    checkNcErr(
        nc_put_att_text(m_ncId, m_varTimeId,
                        "units",
                        strlen("seconds"),
                        "seconds"),
        "time units");

    checkNcErr(
        nc_put_att_text(m_ncId, m_varTimeId,
                        "standard_name",
                        strlen("time"),
                        "time"),
        "time standard_name");

    // x
    checkNcErr(
        nc_put_att_text(m_ncId, m_varXId,
                        "units",
                        strlen("m"),
                        "m"),
        "x units");

    checkNcErr(
        nc_put_att_text(m_ncId, m_varXId,
                        "axis",
                        strlen("X"),
                        "X"),
        "x axis");

    if (m_is2D) {
        checkNcErr(
            nc_put_att_text(m_ncId, m_varYId,
                            "units",
                            strlen("m"),
                            "m"),
            "y units");

        checkNcErr(
            nc_put_att_text(m_ncId, m_varYId,
                            "axis",
                            strlen("Y"),
                            "Y"),
            "y axis");
    }

    // bathymetry
    checkNcErr(
        nc_put_att_text(m_ncId, m_varBId,
                        "coordinates",
                        strlen("y x"),
                        "y x"),
        "b coords");

    // height
    checkNcErr(
        nc_put_att_text(m_ncId, m_varHId,
                        "coordinates",
                        strlen("time y x"),
                        "time y x"),
        "h coords");

    // hu
    checkNcErr(
        nc_put_att_text(m_ncId, m_varHuId,
                        "coordinates",
                        strlen("time y x"),
                        "time y x"),
        "hu coords");

    if (m_is2D) {
        checkNcErr(
            nc_put_att_text(m_ncId, m_varHvId,
                            "coordinates",
                            strlen("time y x"),
                            "time y x"),
            "hv coords");
    }

    std::cout << "nx=" << m_nxOut << " ny=" << m_nyOut << std::endl;
    std::cout
    << "expected b = "
    << (size_t)m_nxOut * m_nyOut
    << "\nactual b = "
    << i_b.size()
    << std::endl;

    //----------------------------------------------------------
    // leave define mode
    //----------------------------------------------------------
    checkNcErr(
        nc_enddef(m_ncId),
        "nc_enddef");

    //----------------------------------------------------------
    // x coordinates
    //----------------------------------------------------------
    std::vector<float> x(m_nxOut);

    for (t_idx ix = 0; ix < m_nxOut; ++ix)
        x[ix] = (ix + 0.5f) * m_dxyOut;

    checkNcErr(
        nc_put_var_float(
            m_ncId,
            m_varXId,
            x.data()),
        "write x");

    //----------------------------------------------------------
    // y coordinates
    //----------------------------------------------------------
    if (m_is2D) {

        std::vector<float> y(m_nyOut);

        for (t_idx iy = 0; iy < m_nyOut; ++iy)
            y[iy] = (iy + 0.5f) * m_dxyOut;

        checkNcErr(
            nc_put_var_float(
                m_ncId,
                m_varYId,
                y.data()),
            "write y");
    }

    //----------------------------------------------------------
    // write bathymetry
    //----------------------------------------------------------
    checkNcErr(
        nc_put_var_float(
            m_ncId,
            m_varBId,
            i_b.data()),
        "write bathymetry");
}

tsunami_lab::io::NetCdfAdaptiveGrid::~NetCdfAdaptiveGrid() {
    if (m_ncId >= 0) {
        checkNcErr(
            nc_close(m_ncId),
            "nc_close");

        m_ncId = -1;
    }
}

void tsunami_lab::io::NetCdfAdaptiveGrid::write(
    tsunami_lab::t_real i_simTime,
    const std::vector<tsunami_lab::t_real>& i_h,
    const std::vector<tsunami_lab::t_real>& i_hu,
    const std::vector<tsunami_lab::t_real>& i_hv
) {
    //----------------------------------------------------------
    // Write simulation time
    //----------------------------------------------------------
    {
        size_t start[1] = {m_timeStep};
        size_t count[1] = {1};

        float time = static_cast<float>(i_simTime);

        checkNcErr(
            nc_put_vara_float(
                m_ncId,
                m_varTimeId,
                start,
                count,
                &time),
            "write time");
    }

    //----------------------------------------------------------
    // Write solution fields
    //----------------------------------------------------------
    if (m_is2D) {

        size_t start[3] = {
            m_timeStep,
            0,
            0
        };

        size_t count[3] = {
            1,
            static_cast<size_t>(m_nyOut),
            static_cast<size_t>(m_nxOut)
        };

        checkNcErr(
            nc_put_vara_float(
                m_ncId,
                m_varHId,
                start,
                count,
                i_h.data()),
            "write h");

        checkNcErr(
            nc_put_vara_float(
                m_ncId,
                m_varHuId,
                start,
                count,
                i_hu.data()),
            "write hu");

        checkNcErr(
            nc_put_vara_float(
                m_ncId,
                m_varHvId,
                start,
                count,
                i_hv.data()),
            "write hv");
    }
    else {

        size_t start[2] = {
            m_timeStep,
            0
        };

        size_t count[2] = {
            1,
            static_cast<size_t>(m_nxOut)
        };

        checkNcErr(
            nc_put_vara_float(
                m_ncId,
                m_varHId,
                start,
                count,
                i_h.data()),
            "write h");

        checkNcErr(
            nc_put_vara_float(
                m_ncId,
                m_varHuId,
                start,
                count,
                i_hu.data()),
            "write hu");
    }

    //----------------------------------------------------------
    // Flush file and advance timestep
    //----------------------------------------------------------
    ++m_timeStep;

    checkNcErr(
        nc_sync(m_ncId),
        "nc_sync");
}