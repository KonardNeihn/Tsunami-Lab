/**
 * @author Konard Neihn
 *
 * @section DESCRIPTION
 * Implementation of the COARDS-compliant netCDF writer for adaptive grid simulations.
 *
 */
#include "NetCdfAdaptiveGrid.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>

// Helper to check and display errors
void tsunami_lab::io::NetCdfAdaptiveGrid::checkNcErr(int i_status, const char* i_context) const {
    if (i_status != NC_NOERR) {
        std::cerr << "NetCdf error in '" << i_context << "': " << nc_strerror(i_status) << std::endl;
        std::abort();
    }
}

// Constructor – opens the file and writes the static data
tsunami_lab::io::NetCdfAdaptiveGrid::NetCdfAdaptiveGrid(
    const std::string& i_path,
    tsunami_lab::t_idx i_nx,
    tsunami_lab::t_idx i_ny,
    tsunami_lab::t_real i_dxy,
    tsunami_lab::t_idx i_stride,
    const tsunami_lab::t_real* i_b,
    const std::vector<std::vector<tsunami_lab::t_idx>>& i_gridResolution,
    bool i_isCheckpoint
) : m_nx(i_nx),
    m_ny(i_ny),
    m_stride(i_stride),
    m_dxy(i_dxy),
    m_gridResolutionLevels(i_gridResolution),
    m_is2D(i_ny > 1),
    m_isCheckpoint(i_isCheckpoint) {

    // Determine the maximum resolution level
    m_maxResolutionLevel = 0;
    for (const auto& row : i_gridResolution) {
        for (tsunami_lab::t_idx lev : row) {
            if (lev > m_maxResolutionLevel) {
                m_maxResolutionLevel = lev;
            }
        }
    }

    // Calculate output dimensions (coarsest level)
    m_nxOut = (i_nx + (1 << m_maxResolutionLevel) - 1) / (1 << m_maxResolutionLevel);
    m_nyOut = (i_ny + (1 << m_maxResolutionLevel) - 1) / (1 << m_maxResolutionLevel);

    int l_status;

    // 1. Create the file (overwrite if it already exists)
    if (!m_isCheckpoint) {
        l_status = nc_create(i_path.c_str(), NC_CLOBBER | NC_NETCDF4, &m_ncId);
        checkNcErr(l_status, "nc_create");

        // 2. Global attributes (COARDS requires conventions)
        const char* l_conventions = "COARDS";
        l_status = nc_put_att_text(m_ncId, NC_GLOBAL, "Conventions", std::strlen(l_conventions), l_conventions);
        checkNcErr(l_status, "put_att Conventions");

        const char* l_title = "Tsunami-Lab Water Simulation with Adaptive Grid";
        l_status = nc_put_att_text(m_ncId, NC_GLOBAL, "title", std::strlen(l_title), l_title);
        checkNcErr(l_status, "put_att title");

        // 3. Define dimensions
        l_status = nc_def_dim(m_ncId, "time", NC_UNLIMITED, &m_dimTimeId);
        checkNcErr(l_status, "def_dim time");

        l_status = nc_def_dim(m_ncId, "x", static_cast<size_t>(m_nxOut), &m_dimXId);
        checkNcErr(l_status, "def_dim x");

        if (m_is2D) {
            l_status = nc_def_dim(m_ncId, "y", static_cast<size_t>(m_nyOut), &m_dimYId);
            checkNcErr(l_status, "def_dim y");
        }

        l_status = nc_def_dim(m_ncId, "resolution_level", static_cast<size_t>(m_maxResolutionLevel + 1), &m_dimResolutionId);
        checkNcErr(l_status, "def_dim resolution_level");

        // 4. Define coordinate variables
        // --- time ---
        {
            int l_dimIds[1] = {m_dimTimeId};
            l_status = nc_def_var(m_ncId, "time", NC_FLOAT, 1, l_dimIds, &m_varTimeId);
            checkNcErr(l_status, "def_var time");

            const char* l_timeUnits = "seconds since the earthquake event";
            l_status = nc_put_att_text(m_ncId, m_varTimeId, "units", std::strlen(l_timeUnits), l_timeUnits);
            checkNcErr(l_status, "put_att time units");

            const char* l_timeStdName = "time";
            l_status = nc_put_att_text(m_ncId, m_varTimeId, "standard_name", std::strlen(l_timeStdName), l_timeStdName);
            checkNcErr(l_status, "put_att time standard_name");
        }

        // --- x coordinate ---
        {
            int l_dimIds[1] = {m_dimXId};
            l_status = nc_def_var(m_ncId, "x", NC_FLOAT, 1, l_dimIds, &m_varXId);
            checkNcErr(l_status, "def_var x");

            const char* l_xUnits = "meters";
            l_status = nc_put_att_text(m_ncId, m_varXId, "units", std::strlen(l_xUnits), l_xUnits);
            checkNcErr(l_status, "put_att x units");

            const char* l_xLongName = "x-coordinate of cell centers";
            l_status = nc_put_att_text(m_ncId, m_varXId, "long_name", std::strlen(l_xLongName), l_xLongName);
            checkNcErr(l_status, "put_att x long_name");

            const char* l_xAxis = "X";
            l_status = nc_put_att_text(m_ncId, m_varXId, "axis", std::strlen(l_xAxis), l_xAxis);
            checkNcErr(l_status, "put_att x axis");
        }

        // --- y coordinate ---
        if (m_is2D) {
            int l_dimIds[1] = {m_dimYId};
            l_status = nc_def_var(m_ncId, "y", NC_FLOAT, 1, l_dimIds, &m_varYId);
            checkNcErr(l_status, "def_var y");

            const char* l_yUnits = "meters";
            l_status = nc_put_att_text(m_ncId, m_varYId, "units", std::strlen(l_yUnits), l_yUnits);
            checkNcErr(l_status, "put_att y units");

            const char* l_yLongName = "y-coordinate of cell centers";
            l_status = nc_put_att_text(m_ncId, m_varYId, "long_name", std::strlen(l_yLongName), l_yLongName);
            checkNcErr(l_status, "put_att y long_name");

            const char* l_yAxis = "Y";
            l_status = nc_put_att_text(m_ncId, m_varYId, "axis", std::strlen(l_yAxis), l_yAxis);
            checkNcErr(l_status, "put_att y axis");
        }

        // --- resolution level ---
        {
            int l_dimIds[1] = {m_dimResolutionId};
            l_status = nc_def_var(m_ncId, "resolution_level", NC_INT, 1, l_dimIds, &m_varResolutionId);
            checkNcErr(l_status, "def_var resolution_level");

            const char* l_resLongName = "Grid resolution level";
            l_status = nc_put_att_text(m_ncId, m_varResolutionId, "long_name", std::strlen(l_resLongName), l_resLongName);
            checkNcErr(l_status, "put_att resolution_level long_name");
        }

        // 5. Define data variables
        int l_dataDimIds[4]; // time, y, x, resolution_level
        int l_nDims;
        if (m_is2D) {
            l_dataDimIds[0] = m_dimTimeId;
            l_dataDimIds[1] = m_dimYId;
            l_dataDimIds[2] = m_dimXId;
            l_dataDimIds[3] = m_dimResolutionId;
            l_nDims = 4;
        } else {
            l_dataDimIds[0] = m_dimTimeId;
            l_dataDimIds[1] = m_dimXId;
            l_dataDimIds[2] = m_dimResolutionId;
            l_nDims = 3;
        }

        // Bathymetry b (time-independent)
        {
            int l_bDimIds[3];
            int l_bNDims;
            if (m_is2D) {
                l_bDimIds[0] = m_dimYId;
                l_bDimIds[1] = m_dimXId;
                l_bDimIds[2] = m_dimResolutionId;
                l_bNDims = 3;
            } else {
                l_bDimIds[0] = m_dimXId;
                l_bDimIds[1] = m_dimResolutionId;
                l_bNDims = 2;
            }

            l_status = nc_def_var(m_ncId, "b", NC_FLOAT, l_bNDims, l_bDimIds, &m_varBId);
            checkNcErr(l_status, "def_var b");

            const char* l_bLongName = "bathymetry (seafloor elevation)";
            l_status = nc_put_att_text(m_ncId, m_varBId, "long_name", std::strlen(l_bLongName), l_bLongName);
            checkNcErr(l_status, "put_att b long_name");

            const char* l_bUnits = "meters";
            l_status = nc_put_att_text(m_ncId, m_varBId, "units", std::strlen(l_bUnits), l_bUnits);
            checkNcErr(l_status, "put_att b units");
        }

        // Water height h
        {
            l_status = nc_def_var(m_ncId, "h", NC_FLOAT, l_nDims, l_dataDimIds, &m_varHId);
            checkNcErr(l_status, "def_var h");

            const char* l_hLongName = "water height above bathymetry";
            l_status = nc_put_att_text(m_ncId, m_varHId, "long_name", std::strlen(l_hLongName), l_hLongName);
            checkNcErr(l_status, "put_att h long_name");

            const char* l_hUnits = "meters";
            l_status = nc_put_att_text(m_ncId, m_varHId, "units", std::strlen(l_hUnits), l_hUnits);
            checkNcErr(l_status, "put_att h units");
        }

        // X-momentum hu
        {
            l_status = nc_def_var(m_ncId, "hu", NC_FLOAT, l_nDims, l_dataDimIds, &m_varHuId);
            checkNcErr(l_status, "def_var hu");

            const char* l_huLongName = "x-momentum (h * u)";
            l_status = nc_put_att_text(m_ncId, m_varHuId, "long_name", std::strlen(l_huLongName), l_huLongName);
            checkNcErr(l_status, "put_att hu long_name");

            const char* l_huUnits = "meters^2 / second";
            l_status = nc_put_att_text(m_ncId, m_varHuId, "units", std::strlen(l_huUnits), l_huUnits);
            checkNcErr(l_status, "put_att hu units");
        }

        // Y-momentum hv (2D only)
        if (m_is2D) {
            l_status = nc_def_var(m_ncId, "hv", NC_FLOAT, l_nDims, l_dataDimIds, &m_varHvId);
            checkNcErr(l_status, "def_var hv");

            const char* l_hvLongName = "y-momentum (h * v)";
            l_status = nc_put_att_text(m_ncId, m_varHvId, "long_name", std::strlen(l_hvLongName), l_hvLongName);
            checkNcErr(l_status, "put_att hv long_name");

            const char* l_hvUnits = "meters^2 / second";
            l_status = nc_put_att_text(m_ncId, m_varHvId, "units", std::strlen(l_hvUnits), l_hvUnits);
            checkNcErr(l_status, "put_att hv units");
        }

        // 6. Leave define mode
        l_status = nc_enddef(m_ncId);
        checkNcErr(l_status, "nc_enddef");

        // 7. Write coordinate arrays (x, y)
        {
            std::vector<float> l_xCoords(m_nxOut);
            for (tsunami_lab::t_idx l_ox = 0; l_ox < m_nxOut; ++l_ox) {
                l_xCoords[l_ox] = static_cast<float>((l_ox + 0.5) * (1 << m_maxResolutionLevel) * m_dxy);
            }
            l_status = nc_put_var_float(m_ncId, m_varXId, l_xCoords.data());
            checkNcErr(l_status, "put_var x");
        }

        if (m_is2D) {
            std::vector<float> l_yCoords(m_nyOut);
            for (tsunami_lab::t_idx l_oy = 0; l_oy < m_nyOut; ++l_oy) {
                l_yCoords[l_oy] = static_cast<float>((l_oy + 0.5) * (1 << m_maxResolutionLevel) * m_dxy);
            }
            l_status = nc_put_var_float(m_ncId, m_varYId, l_yCoords.data());
            checkNcErr(l_status, "put_var y");
        }

        // 8. Write resolution levels
        {
            std::vector<int> l_resolutionLevels(m_maxResolutionLevel + 1);
            for (tsunami_lab::t_idx lev = 0; lev <= m_maxResolutionLevel; ++lev) {
                l_resolutionLevels[lev] = static_cast<int>(lev);
            }
            l_status = nc_put_var_int(m_ncId, m_varResolutionId, l_resolutionLevels.data());
            checkNcErr(l_status, "put_var resolution_level");
        }

        // 9. Write bathymetry (static – averaged over each cell block)
        {
            const size_t l_totalOut = static_cast<size_t>(m_nxOut) * static_cast<size_t>(m_is2D ? m_nyOut : 1);
            std::vector<float> l_buf(l_totalOut, 0.0f);

            if (m_is2D) {
                for (tsunami_lab::t_idx l_oy = 0; l_oy < m_nyOut; ++l_oy) {
                    for (tsunami_lab::t_idx l_ox = 0; l_ox < m_nxOut; ++l_ox) {
                        // Determine the resolution level for this block
                        tsunami_lab::t_idx l_resLevel = 0;
                        for (tsunami_lab::t_idx l_iy = l_oy * (1 << m_maxResolutionLevel);
                             l_iy < std::min((l_oy + 1) * (1 << m_maxResolutionLevel), m_ny);
                             ++l_iy) {
                            for (tsunami_lab::t_idx l_ix = l_ox * (1 << m_maxResolutionLevel);
                                 l_ix < std::min((l_ox + 1) * (1 << m_maxResolutionLevel), m_nx);
                                 ++l_ix) {
                                if (m_gridResolutionLevels[l_iy][l_ix] > l_resLevel) {
                                    l_resLevel = m_gridResolutionLevels[l_iy][l_ix];
                                }
                            }
                        }

                        // Average bathymetry over the block
                        float l_sum = 0.0f;
                        int l_count = 0;
                        for (tsunami_lab::t_idx l_iy = l_oy * (1 << m_maxResolutionLevel);
                             l_iy < std::min((l_oy + 1) * (1 << m_maxResolutionLevel), m_ny);
                             ++l_iy) {
                            for (tsunami_lab::t_idx l_ix = l_ox * (1 << m_maxResolutionLevel);
                                 l_ix < std::min((l_ox + 1) * (1 << m_maxResolutionLevel), m_nx);
                                 ++l_ix) {
                                l_sum += static_cast<float>(i_b[l_iy * m_stride + l_ix]);
                                ++l_count;
                            }
                        }
                        l_buf[l_oy * m_nxOut + l_ox] = l_sum / static_cast<float>(l_count);
                    }
                }
            } else {
                for (tsunami_lab::t_idx l_ox = 0; l_ox < m_nxOut; ++l_ox) {
                    // Determine the resolution level for this block
                    tsunami_lab::t_idx l_resLevel = 0;
                    for (tsunami_lab::t_idx l_ix = l_ox * (1 << m_maxResolutionLevel);
                         l_ix < std::min((l_ox + 1) * (1 << m_maxResolutionLevel), m_nx);
                         ++l_ix) {
                        if (m_gridResolutionLevels[l_ix][0] > l_resLevel) {
                            l_resLevel = m_gridResolutionLevels[l_ix][0];
                        }
                    }

                    // Average bathymetry over the block
                    float l_sum = 0.0f;
                    int l_count = 0;
                    for (tsunami_lab::t_idx l_ix = l_ox * (1 << m_maxResolutionLevel);
                         l_ix < std::min((l_ox + 1) * (1 << m_maxResolutionLevel), m_nx);
                         ++l_ix) {
                        l_sum += static_cast<float>(i_b[l_ix]);
                        ++l_count;
                    }
                    l_buf[l_ox] = l_sum / static_cast<float>(l_count);
                }
            }

            // Write bathymetry for each resolution level
            for (tsunami_lab::t_idx lev = 0; lev <= m_maxResolutionLevel; ++lev) {
                size_t l_start[3] = {0, 0, lev};
                size_t l_count[3];
                if (m_is2D) {
                    l_count[0] = m_nyOut;
                    l_count[1] = m_nxOut;
                    l_count[2] = 1;
                } else {
                    l_count[0] = m_nxOut;
                    l_count[1] = 1;
                }
                l_status = nc_put_vara_float(m_ncId, m_varBId, l_start, l_count, l_buf.data());
                checkNcErr(l_status, "put_vara b");
            }
        }
    } else {
        // Open existing checkpoint file
        l_status = nc_open(i_path.c_str(), NC_WRITE, &m_ncId);
        checkNcErr(l_status, "nc_open");

        // Load existing variable IDs
        l_status = nc_inq_varid(m_ncId, "time", &m_varTimeId);
        checkNcErr(l_status, "inq_varid time");

        l_status = nc_inq_varid(m_ncId, "h", &m_varHId);
        checkNcErr(l_status, "inq_varid h");

        l_status = nc_inq_varid(m_ncId, "hu", &m_varHuId);
        checkNcErr(l_status, "inq_varid hu");

        if (m_is2D) {
            l_status = nc_inq_varid(m_ncId, "hv", &m_varHvId);
            checkNcErr(l_status, "inq_varid hv");
        }

        // Determine the current time step
        l_status = nc_inq_dimid(m_ncId, "time", &m_dimTimeId);
        checkNcErr(l_status, "inq_dimid time");

        size_t l_timeLen = 0;
        l_status = nc_inq_dimlen(m_ncId, m_dimTimeId, &l_timeLen);
        checkNcErr(l_status, "inq_dimlen time");

        m_timeStep = static_cast<tsunami_lab::t_idx>(l_timeLen);
    }
}

// Destructor
tsunami_lab::io::NetCdfAdaptiveGrid::~NetCdfAdaptiveGrid() {
    if (m_ncId >= 0) {
        nc_close(m_ncId);
        m_ncId = -1;
    }
}

// write() – appends one averaged time record
void tsunami_lab::io::NetCdfAdaptiveGrid::write(
    tsunami_lab::t_real i_simTime,
    const tsunami_lab::t_real* i_h,
    const tsunami_lab::t_real* i_hu,
    const tsunami_lab::t_real* i_hv
) {
    int l_status;


    // 1. Append the simulation time
    {
        size_t l_start[1] = {m_timeStep};
        size_t l_count[1] = {1};
        float l_t = static_cast<float>(i_simTime);
        l_status = nc_put_vara_float(m_ncId, m_varTimeId, l_start, l_count, &l_t);
        checkNcErr(l_status, "put_vara time");
    }


    // 2. Prepare buffers for output data
    const size_t l_nxOut = static_cast<size_t>(m_nxOut);
    const size_t l_nyOut = static_cast<size_t>(m_is2D ? m_nyOut : 1);
    const size_t l_total = l_nxOut * l_nyOut;

    std::vector<float> l_hBuf(l_total, 0.0f);
    std::vector<float> l_huBuf(l_total, 0.0f);
    std::vector<float> l_hvBuf;
    if (m_is2D && i_hv != nullptr) {
        l_hvBuf.assign(l_total, 0.0f);
    }


    // 3. Average data over blocks and write for each resolution level
    for (tsunami_lab::t_idx lev = 0; lev <= m_maxResolutionLevel; ++lev) {
        if (m_is2D) {

            for (tsunami_lab::t_idx l_oy = 0; l_oy < m_nyOut; ++l_oy) {

                for (tsunami_lab::t_idx l_ox = 0; l_ox < m_nxOut; ++l_ox) {

                    // Check if this block has the current resolution level
                    bool l_hasLevel = false;
                    for (tsunami_lab::t_idx l_iy = l_oy * (1 << (m_maxResolutionLevel - lev));
                         l_iy < std::min((l_oy + 1) * (1 << (m_maxResolutionLevel - lev)), m_ny);
                         ++l_iy) {
                            
                        for (tsunami_lab::t_idx l_ix = l_ox * (1 << (m_maxResolutionLevel - lev));
                             l_ix < std::min((l_ox + 1) * (1 << (m_maxResolutionLevel - lev)), m_nx);
                             ++l_ix) {

                            if (m_gridResolutionLevels[l_iy][l_ix] == lev) {

                                l_hasLevel = true;
                                break;
                            }
                        }

                        if (l_hasLevel) break;
                    }

                    if (!l_hasLevel) continue;

                    // Average data over the block
                    float l_sumH = 0.0f;
                    float l_sumHu = 0.0f;
                    float l_sumHv = 0.0f;
                    int l_count = 0;

                    for (tsunami_lab::t_idx l_iy = l_oy * (1 << (m_maxResolutionLevel - lev));
                         l_iy < std::min((l_oy + 1) * (1 << (m_maxResolutionLevel - lev)), m_ny);
                         ++l_iy) {
                        for (tsunami_lab::t_idx l_ix = l_ox * (1 << (m_maxResolutionLevel - lev));
                             l_ix < std::min((l_ox + 1) * (1 << (m_maxResolutionLevel - lev)), m_nx);
                             ++l_ix) {
                            size_t l_idx = static_cast<size_t>(l_iy) * m_stride + static_cast<size_t>(l_ix);
                            l_sumH += static_cast<float>(i_h[l_idx]);
                            l_sumHu += static_cast<float>(i_hu[l_idx]);
                            if (!l_hvBuf.empty()) {
                                l_sumHv += static_cast<float>(i_hv[l_idx]);
                            }
                            ++l_count;
                        }
                    }

                    size_t l_outIdx = static_cast<size_t>(l_oy) * l_nxOut + static_cast<size_t>(l_ox);
                    float l_inv = 1.0f / static_cast<float>(l_count);
                    l_hBuf[l_outIdx] = l_sumH * l_inv;
                    l_huBuf[l_outIdx] = l_sumHu * l_inv;
                    if (!l_hvBuf.empty()) {
                        l_hvBuf[l_outIdx] = l_sumHv * l_inv;
                    }
                }
            }
        } else {

            for (tsunami_lab::t_idx l_ox = 0; l_ox < m_nxOut; ++l_ox) {
                // Check if this block has the current resolution level
                bool l_hasLevel = false;
                for (tsunami_lab::t_idx l_ix = l_ox * (1 << (m_maxResolutionLevel - lev));
                     l_ix < std::min((l_ox + 1) * (1 << (m_maxResolutionLevel - lev)), m_nx);
                     ++l_ix) {
                    if (m_gridResolutionLevels[0][l_ix] == lev) {
                        l_hasLevel = true;
                        break;
                    }
                }

                if (!l_hasLevel) continue;

                // Average data over the block
                float l_sumH = 0.0f;
                float l_sumHu = 0.0f;
                int l_count = 0;
                for (tsunami_lab::t_idx l_ix = l_ox * (1 << (m_maxResolutionLevel - lev));
                     l_ix < std::min((l_ox + 1) * (1 << (m_maxResolutionLevel - lev)), m_nx);
                     ++l_ix) {
                    l_sumH += static_cast<float>(i_h[l_ix]);
                    l_sumHu += static_cast<float>(i_hu[l_ix]);
                    ++l_count;
                }
                float l_inv = 1.0f / static_cast<float>(l_count);
                l_hBuf[l_ox] = l_sumH * l_inv;
                l_huBuf[l_ox] = l_sumHu * l_inv;
            }
        }


        // Write the averaged record for this resolution level
        if (m_is2D) {
            size_t l_start[4] = {m_timeStep, 0, 0, lev};
            size_t l_count[4] = {1, l_nyOut, l_nxOut, 1};

            l_status = nc_put_vara_float(m_ncId, m_varHId, l_start, l_count, l_hBuf.data());
            checkNcErr(l_status, "put_vara h");

            l_status = nc_put_vara_float(m_ncId, m_varHuId, l_start, l_count, l_huBuf.data());
            checkNcErr(l_status, "put_vara hu");

            if (!l_hvBuf.empty()) {
                l_status = nc_put_vara_float(m_ncId, m_varHvId, l_start, l_count, l_hvBuf.data());
                checkNcErr(l_status, "put_vara hv");
            }
        } else {
            size_t l_start[3] = {m_timeStep, 0, lev};
            size_t l_count[3] = {1, l_nxOut, 1};

            l_status = nc_put_vara_float(m_ncId, m_varHId, l_start, l_count, l_hBuf.data());
            checkNcErr(l_status, "put_vara h");

            l_status = nc_put_vara_float(m_ncId, m_varHuId, l_start, l_count, l_huBuf.data());
            checkNcErr(l_status, "put_vara hu");
        }
    }


    // 4. Advance record index
    ++m_timeStep;
    l_status = nc_sync(m_ncId);
    checkNcErr(l_status, "nc_sync");

}