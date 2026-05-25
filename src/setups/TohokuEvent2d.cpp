/**
 * @author 
 *
 * @section DESCRIPTION
 * One-dimensional dam break problem.
 **/
#include "TohokuEvent2d.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <iostream>

tsunami_lab::setups::TohokuEvent2d::TohokuEvent2d(std::string pathBathymetry, std::string pathDisplacement, t_idx i_nx, t_idx i_ny) {
    m_nx = i_nx;
    m_ny = i_ny;
    
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

    const std::size_t nx = m_x.size();
    const std::size_t ny = m_y.size();
    const std::size_t nx_d = dx.size();
    const std::size_t ny_d = dy.size();

    if (nx == 0 || ny == 0 || nx_d == 0 || ny_d == 0) {
        throw std::runtime_error("TohokuEvent2d: empty grid");
    }

    if (l_bathy.size() != nx * ny) {
        throw std::runtime_error("TohokuEvent2d: bathymetry variable has unexpected shape");
    }
    if (l_disp.size() != nx_d * ny_d) {
        throw std::runtime_error("TohokuEvent2d: displacement variable has unexpected shape");
    }

    m_2dBathymetry.resize(ny, std::vector<t_real>(nx));
    m_2dDisplacement.resize(ny_d, std::vector<t_real>(nx_d));

    std::cout << "Bathymetry size: " << nx << " times " << ny << std::endl;
    std::cout << "Displacement size: " << nx_d << " times " << ny_d << std::endl;
    std::cout << l_bathy.size() << std::endl;

    for (std::size_t j = 0; j < ny; ++j) {
        for (std::size_t i = 0; i < nx; ++i) {
            m_2dBathymetry[j][i] = l_bathy[j * nx + i];
        }
    }

    for (std::size_t j = 0; j < ny_d; ++j) {
        for (std::size_t i = 0; i < nx_d; ++i) {
            m_2dDisplacement[j][i] = l_disp[j * nx_d + i];
        }
    }
}

tsunami_lab::t_real tsunami_lab::setups::TohokuEvent2d::getHeight(t_real i_x, t_real i_y) const {
    float l_indicesPerCellX = m_2dDisplacement.size() / m_nx;
    float l_indicesPerCellY = m_2dDisplacement[0].size() / m_ny;
    std::size_t l_ix = l_indicesPerCellX * i_x;
    std::size_t l_iy = l_indicesPerCellY * i_y;
    if (l_iy >= m_2dDisplacement.size() || l_ix >= m_2dDisplacement[0].size()) {
        throw std::runtime_error("getBathymetry: index out of bounds");
    }
    std::cout << "Height: " << m_2dDisplacement[i_y][i_x] << std::endl;
    return m_2dDisplacement[l_iy][l_ix];
}

tsunami_lab::t_real tsunami_lab::setups::TohokuEvent2d::getMomentumX( t_real,
                                                                   t_real ) const {
  return 0;
}

tsunami_lab::t_real tsunami_lab::setups::TohokuEvent2d::getMomentumY( t_real,
                                                                   t_real ) const {
  return 0;
}

tsunami_lab::t_real tsunami_lab::setups::TohokuEvent2d::getBathymetry( t_real i_x,
                                                                            t_real i_y ) const {

    float l_indicesPerCellX = m_2dBathymetry.size() / m_nx;
    float l_indicesPerCellY = m_2dBathymetry[0].size() / m_ny;
    std::size_t l_ix = l_indicesPerCellX * i_x;
    std::size_t l_iy = l_indicesPerCellY * i_y;
    if (l_iy >= m_2dBathymetry.size() || l_ix >= m_2dBathymetry[0].size()) {
        throw std::runtime_error("getBathymetry: index out of bounds");
    }
    std::cout << "Bathymetry: " << m_2dBathymetry[l_iy][l_ix] << std::endl;
    return m_2dBathymetry[l_iy][l_ix];
}