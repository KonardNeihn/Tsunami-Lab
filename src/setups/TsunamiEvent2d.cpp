/**
 * @author 
 *
 * @section DESCRIPTION
 * One-dimensional dam break problem.
 **/
#include "TsunamiEvent2d.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <iostream>

std::size_t tsunami_lab::setups::TsunamiEvent2d::findClosestIndex(
    const std::vector<t_real> &i_coords,
    t_real i_value) const {
  if (i_coords.empty()) return 0;

  std::size_t best = 0;
  t_real bestDiff = std::abs(i_coords[0] - i_value);

  for (std::size_t i = 1; i < i_coords.size(); ++i) {
    t_real diff = std::abs(i_coords[i] - i_value);
    if (diff < bestDiff) {
      best = i;
      bestDiff = diff;
    }
  }

  return best;
}

tsunami_lab::setups::TsunamiEvent2d::TsunamiEvent2d(std::string pathBathymetry, std::string pathDisplacement) {
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
        throw std::runtime_error("TsunamiEvent2d: empty grid");
    }

    if (l_bathy.size() != nx * ny) {
        throw std::runtime_error("TsunamiEvent2d: bathymetry variable has unexpected shape");
    }
    if (l_disp.size() != nx_d * ny_d) {
        throw std::runtime_error("TsunamiEvent2d: displacement variable has unexpected shape");
    }

    // Build a bathymetry grid on the displacement coordinate system.
    // The displacement file is 100x100 while the bathymetry file is 1000x1000.
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

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getHeight(t_real i_x, t_real i_y) const {
    // hardcoded 100x100, so no next index has to be found
    return m_2dDisplacement[i_y][i_x];
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getMomentumX( t_real,
                                                                   t_real ) const {
  return 0;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getMomentumY( t_real,
                                                                   t_real ) const {
  return 0;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getBathymetry( t_real i_x,
                                                                            t_real i_y ) const {
    std::size_t l_ix = 10 * i_x;
    std::size_t l_iy = 10 * i_y;
    if (l_iy >= m_2dBathymetry.size() || l_ix >= m_2dBathymetry[0].size()) {
        throw std::runtime_error("getBathymetry: index out of bounds");
    }
    return m_2dBathymetry[l_iy][l_ix];
}