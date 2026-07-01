/**
 * @author 
 *
 * @section DESCRIPTION
 * Tsunami.
 **/
#include "TsunamiEvent2d.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <iostream>

tsunami_lab::setups::TsunamiEvent2d::TsunamiEvent2d(
    Config& config,
    std::string pathBathymetry,
    std::string pathDisplacement)
:g_config(config)
{
    tsunami_lab::io::NetCdfReader l_ncBath;
    tsunami_lab::io::NetCdfReader l_ncDisp;

    m_x = l_ncBath.read(pathBathymetry, "x");
    m_y = l_ncBath.read(pathBathymetry, "y");

    auto l_bathy = l_ncBath.read(pathBathymetry, "z");

    std::vector<t_real> dx = l_ncDisp.read(pathDisplacement, "x");
    std::vector<t_real> dy = l_ncDisp.read(pathDisplacement, "y");

    auto l_disp = l_ncDisp.read(pathDisplacement, "z");

    m_xd = dx;
    m_yd = dy;

    g_config.is_2d = true;
    g_config.ny = static_cast<tsunami_lab::t_idx>(g_config.nx * (getDomainLength() / getDomainWidth()));

    g_config.width = getDomainWidth();
    g_config.dxy = getDomainWidth() / g_config.nx;
    g_config.domainStartX = std::min(m_x.front(), dx.front());
    g_config.domainStartY = std::min(m_y.front(), dy.front());

    const std::size_t nx   = m_x.size();
    const std::size_t ny   = m_y.size();
    const std::size_t nx_d = dx.size();
    const std::size_t ny_d = dy.size();

    if (nx == 0 || ny == 0 || nx_d == 0 || ny_d == 0) {
        throw std::runtime_error("TsunamiEvent2d: empty grid");
    }

    if (l_bathy.size() != nx * ny) {
        throw std::runtime_error("TsunamiEvent2d: bathymetry shape mismatch");
    }

    if (l_disp.size() != nx_d * ny_d) {
        throw std::runtime_error("TsunamiEvent2d: displacement shape mismatch");
    }

    m_2dBathymetry.resize(ny,   std::vector<t_real>(nx));
    m_2dDisplacement.resize(ny_d, std::vector<t_real>(nx_d));

    for (std::size_t j = 0; j < ny; ++j)
        for (std::size_t i = 0; i < nx; ++i)
            m_2dBathymetry[j][i] = l_bathy[j * nx + i];

    for (std::size_t j = 0; j < ny_d; ++j)
        for (std::size_t i = 0; i < nx_d; ++i)
            m_2dDisplacement[j][i] = l_disp[j * nx_d + i];
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getHeight(t_real solverX, t_real solverY) const
{
    t_real physicalX = g_config.domainStartX + (solverX + 0.5) * g_config.dxy;
    t_real physicalY = g_config.domainStartY + (solverY + 0.5) * g_config.dxy;

    // der nähere wirklich existierende Index wird genommen
    auto itX = std::lower_bound(m_xd.begin(), m_xd.end(), physicalX);
    auto itY = std::lower_bound(m_yd.begin(), m_yd.end(), physicalY);

    std::size_t existingPhysicalX = std::distance(m_xd.begin(), itX);
    std::size_t existingPhysicalY = std::distance(m_yd.begin(), itY);

    if (existingPhysicalX >= m_xd.size()) existingPhysicalX = m_xd.size() - 1;
    if (existingPhysicalY >= m_yd.size()) existingPhysicalY = m_yd.size() - 1;

    t_real b = getBathymetry(solverX, solverY);
    t_real h = m_2dDisplacement[existingPhysicalY][existingPhysicalX];

    return std::max(-b + h, (t_real)0);
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getMomentumX( t_real, t_real ) const {
  return 0;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getMomentumY( t_real, t_real ) const {
  return 0;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getBathymetry(t_real solverX, t_real solverY) const
{
    t_real physicalX = g_config.domainStartX + (solverX + 0.5) * g_config.dxy;
    t_real physicalY = g_config.domainStartY + (solverY + 0.5) * g_config.dxy;

    // der nähere wirklich existierende Index wird genommen
    auto itX = std::lower_bound(m_x.begin(), m_x.end(), physicalX);
    auto itY = std::lower_bound(m_y.begin(), m_y.end(), physicalY);

    std::size_t existingPhysicalX = std::distance(m_x.begin(), itX);
    std::size_t existingPhysicalY = std::distance(m_y.begin(), itY);

    if (existingPhysicalX >= m_xd.size()) existingPhysicalX = m_xd.size() - 1;
    if (existingPhysicalY >= m_yd.size()) existingPhysicalY = m_yd.size() - 1;

    t_real b = m_2dBathymetry[existingPhysicalY][existingPhysicalX];

    return b;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getDomainWidth() const {
    return std::min(m_x.back(), m_xd.back()) - std::max(m_x.front(), m_xd.front());
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getDomainLength() const {
    return std::min(m_y.back(), m_yd.back()) - std::max(m_y.front(), m_yd.front());
}

tsunami_lab::t_idx tsunami_lab::setups::TsunamiEvent2d::getResolution(tsunami_lab::t_real i_x, tsunami_lab::t_real) const {
    tsunami_lab::t_real center = g_config.domainStartX + (g_config.width / 2.0);
    if (i_x > center) {
        return 2;
    }

    return 1;
}