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

std::size_t tsunami_lab::setups::TsunamiEvent2d::findClosestIndex(const std::vector<t_real> &i_coords,
                                                                   t_real i_value) const {
  if (i_coords.empty()) return 0;
  if (i_value <= i_coords.front()) return 0;
  if (i_value >= i_coords.back()) return i_coords.size() - 1;

  auto it = std::lower_bound(i_coords.begin(), i_coords.end(), i_value);
  if (it == i_coords.begin()) return 0;

  auto prev = it - 1;
  auto idx = static_cast<std::size_t>(std::distance(i_coords.begin(), it));
  auto prevIdx = idx - 1;

  t_real diffNext = std::abs(*it - i_value);
  t_real diffPrev = std::abs(*prev - i_value);
  return (diffPrev <= diffNext) ? prevIdx : idx;
}

tsunami_lab::setups::TsunamiEvent2d::TsunamiEvent2d(std::string pathBathymetry, std::string pathDisplacement) {
    tsunami_lab::io::NetCdf l_ncBath(pathBathymetry, 0, 0, 0, 0, nullptr);
    tsunami_lab::io::NetCdf l_ncDisp(pathDisplacement, 0, 0, 0, 0, nullptr);

    m_x = l_ncBath.read(pathBathymetry, "x");
    m_y = l_ncBath.read(pathBathymetry, "y");

    m_1dBathymetry = l_ncBath.read(pathBathymetry, "z");
    m_1dDisplacement = l_ncDisp.read(pathDisplacement, "z");
    
    std::size_t l_nx = m_x.size();
    std::size_t l_ny = m_y.size();

    if (l_nx == 0 || l_ny == 0) {
      throw std::runtime_error("TsunamiEvent2d: empty x or y coordinate vector");
    }
    if (m_1dBathymetry.size() != l_nx * l_ny || m_1dDisplacement.size() != l_nx * l_ny) {
      throw std::runtime_error("TsunamiEvent2d: unexpected z variable shape");
    }

    m_2dBathymetry.assign(l_ny, std::vector<t_real>(l_nx));
    m_2dDisplacement.assign(l_ny, std::vector<t_real>(l_nx));

    for (std::size_t j = 0; j < l_ny; ++j) {
        for (std::size_t i = 0; i < l_nx; ++i) {
            std::size_t idx = j * l_nx + i;
            m_2dBathymetry[j][i] = m_1dBathymetry[idx];
            m_2dDisplacement[j][i] = m_1dDisplacement[idx];
        }
    }
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getHeight( t_real i_x,
                                                                    t_real i_y ) const {
    std::size_t l_ix = findClosestIndex(m_x, i_x);
    std::size_t l_iy = findClosestIndex(m_y, i_y);
    return m_2dDisplacement[l_iy][l_ix];
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
    std::size_t l_ix = findClosestIndex(m_x, i_x);
    std::size_t l_iy = findClosestIndex(m_y, i_y);
    return m_2dBathymetry[l_iy][l_ix];
}