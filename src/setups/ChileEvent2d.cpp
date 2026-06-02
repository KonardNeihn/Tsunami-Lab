/**
 * @author 
 *
 * @section DESCRIPTION
 * Chile 2010 Tsunami.
 **/
#include "ChileEvent2d.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <iostream>

tsunami_lab::setups::ChileEvent2d::ChileEvent2d(
    Config& config,
    std::string pathBathymetry,
    std::string pathDisplacement)
{

    tsunami_lab::io::NetCdfReader l_ncBath;
    tsunami_lab::io::NetCdfReader l_ncDisp;

    m_x = l_ncBath.read(pathBathymetry, "x");
    m_y = l_ncBath.read(pathBathymetry, "y");

    std::cout << "Bathymetry X range: "
          << m_x.front() << " -> "
          << m_x.back() << std::endl;

    std::cout << "Bathymetry Y range: "
          << m_y.front() << " -> "
          << m_y.back() << std::endl;

    auto l_bathy = l_ncBath.read(pathBathymetry, "z");

    std::vector<t_real> dx = l_ncDisp.read(pathDisplacement, "x");
    std::vector<t_real> dy = l_ncDisp.read(pathDisplacement, "y");

    std::cout << "Displacement X range: "
          << dx.front() << " -> "
          << dx.back() << std::endl;

    std::cout << "Displacement Y range: "
          << dy.front() << " -> "
          << dy.back() << std::endl;

    auto l_disp = l_ncDisp.read(pathDisplacement, "z");


    std::cout << "Bathymetry X width: "
          << m_x.back() - m_x.front() << std::endl;

    std::cout << "Bathymetry Y width: "
          << m_y.back() - m_y.front() << std::endl;

    std::cout << "Displacement X width: "
          << dx.back() - dx.front() << std::endl;

    std::cout << "Displacement Y width: "
          << dy.back() - dy.front() << std::endl;

    config.is_2d = true;
    config.ny = config.nx * (getDomainLength() / getDomainWidth());
    config.width = getDomainWidth();
    config.dxy = config.width / config.nx;

    m_xd = dx;
    m_yd = dy;

    const std::size_t nx   = m_x.size();
    const std::size_t ny   = m_y.size();
    const std::size_t nx_d = dx.size();
    const std::size_t ny_d = dy.size();

    if (nx == 0 || ny == 0 || nx_d == 0 || ny_d == 0) {
        throw std::runtime_error("ChileEvent2d: empty grid");
    }

    if (l_bathy.size() != nx * ny) {
        throw std::runtime_error("ChileEvent2d: bathymetry shape mismatch");
    }

    if (l_disp.size() != nx_d * ny_d) {
        throw std::runtime_error("ChileEvent2d: displacement shape mismatch");
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

tsunami_lab::t_real tsunami_lab::setups::ChileEvent2d::getHeight(
    t_real i_x,
    t_real i_y) const
{
    // i_x and i_y are in meters
    // conversion: meters -> dataset indices
    const float scaleX =
        static_cast<float>(m_2dDisplacement[0].size()) /
        getDomainWidth();

    const float scaleY =
        static_cast<float>(m_2dDisplacement.size()) /
        getDomainLength();

    std::size_t l_ix = std::min(
        static_cast<std::size_t>(scaleX * i_x),
        m_2dDisplacement[0].size() - 1
    );

    std::size_t l_iy = std::min(
        static_cast<std::size_t>(scaleY * i_y),
        m_2dDisplacement.size() - 1
    );

    t_real b =
        m_2dBathymetry[l_iy][l_ix];

    t_real disp =
        m_2dDisplacement[l_iy][l_ix];

    // water depth over bathymetry
    t_real h =
        std::max(-b + disp, (t_real)0);

    return h;
}

tsunami_lab::t_real tsunami_lab::setups::ChileEvent2d::getMomentumX( t_real,
                                                                   t_real ) const {
  return 0;
}

tsunami_lab::t_real tsunami_lab::setups::ChileEvent2d::getMomentumY( t_real,
                                                                   t_real ) const {
  return 0;
}

tsunami_lab::t_real tsunami_lab::setups::ChileEvent2d::getBathymetry(
    t_real i_x,
    t_real i_y) const
{
    
    // i_x and i_y are in meters
    // conversion: meters -> dataset indices
    const float scaleX =
        static_cast<float>(m_2dBathymetry[0].size()) /
        getDomainWidth();

    const float scaleY =
        static_cast<float>(m_2dBathymetry.size()) /
        getDomainLength();

    std::size_t l_ix = std::min(
        static_cast<std::size_t>(scaleX * i_x),
        m_2dBathymetry[0].size() - 1
    );

    std::size_t l_iy = std::min(
        static_cast<std::size_t>(scaleY * i_y),
        m_2dBathymetry.size() - 1
    );

    return m_2dBathymetry[l_iy][l_ix];
}

tsunami_lab::t_real tsunami_lab::setups::ChileEvent2d::getDomainWidth() const {
    return m_x.back() - m_x.front();
}

tsunami_lab::t_real tsunami_lab::setups::ChileEvent2d::getDomainLength() const {
    return m_y.back() - m_y.front();
}