/**
 * @author 
 *
 * @section DESCRIPTION
 * 2d Checkpoint setup.
 **/
#include "Checkpoint2d.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <iostream>

tsunami_lab::setups::Checkpoint2d::Checkpoint2d(
    std::string checkpoint)
{
    std::string l_checkpointFolder = "";
    m_checkpointPath = l_checkpointFolder + checkpoint;

    // todo: implement NetCdfCheckpoint class to read (and write) checkpoint files
    tsunami_lab::io::NetCdfCheckpoint l_checkpointReader();
    
}

tsunami_lab::t_real tsunami_lab::setups::TohokuEvent2d::getHeight(
    t_real i_x,
    t_real i_y) const
{
    const float scaleX =
        static_cast<float>(m_2dDisplacement[0].size()) / m_nx;

    const float scaleY =
        static_cast<float>(m_2dDisplacement.size()) / m_ny;

    std::size_t l_ix = std::min(
        static_cast<std::size_t>(scaleX * i_x),
        m_2dDisplacement[0].size() - 1);

    std::size_t l_iy = std::min(
        static_cast<std::size_t>(scaleY * i_y),
        m_2dDisplacement.size() - 1);

    return std::max(-getBathymetry(i_x, i_y), 0.0f) + m_2dDisplacement[l_iy][l_ix];
}

tsunami_lab::t_real tsunami_lab::setups::TohokuEvent2d::getMomentumX( t_real,
                                                                   t_real ) const {
  return 0;
}

tsunami_lab::t_real tsunami_lab::setups::TohokuEvent2d::getMomentumY( t_real,
                                                                   t_real ) const {
  return 0;
}

tsunami_lab::t_real tsunami_lab::setups::TohokuEvent2d::getBathymetry(
    t_real i_x,
    t_real i_y) const
{
    const float scaleX =
        static_cast<float>(m_2dBathymetry[0].size()) / m_nx;

    const float scaleY =
        static_cast<float>(m_2dBathymetry.size()) / m_ny;

    std::size_t l_ix = std::min(
        static_cast<std::size_t>(scaleX * i_x),
        m_2dBathymetry[0].size() - 1);

    std::size_t l_iy = std::min(
        static_cast<std::size_t>(scaleY * i_y),
        m_2dBathymetry.size() - 1);

    return m_2dBathymetry[l_iy][l_ix];
}