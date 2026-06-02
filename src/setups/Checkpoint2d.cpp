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
    std::string checkpoint,
   Config& config)
: g_config(config)
{
    std::string l_checkpointFolder = "";
    m_checkpointPath = l_checkpointFolder + checkpoint;

    tsunami_lab::io::NetCdfCheckpoint l_checkpointReader(g_config);
    
    l_checkpointReader.readCheckpointAndSetParameters(
        m_checkpointPath
    );

    m_2dBathymetry = l_checkpointReader.read2DVariable(m_checkpointPath, "bathymetry");
    m_2dDisplacement = l_checkpointReader.read2DVariable(m_checkpointPath, "height");
    m_2dMomentumX = l_checkpointReader.read2DVariable(m_checkpointPath, "momentumX");
    m_2dMomentumY = l_checkpointReader.read2DVariable(m_checkpointPath, "momentumY");
}

tsunami_lab::t_real tsunami_lab::setups::Checkpoint2d::getHeight(
    t_real i_x,
    t_real i_y) const
{
    // displacement is the actual water height
    return m_2dDisplacement[i_y][i_x];
}

tsunami_lab::t_real tsunami_lab::setups::Checkpoint2d::getMomentumX( t_real i_x,
                                                                   t_real i_y) const {
  return m_2dMomentumX[i_y][i_x];
}

tsunami_lab::t_real tsunami_lab::setups::Checkpoint2d::getMomentumY( t_real i_x,
                                                                   t_real i_y) const {
  return m_2dMomentumY[i_y][i_x];
}

tsunami_lab::t_real tsunami_lab::setups::Checkpoint2d::getBathymetry(
    t_real i_x,
    t_real i_y) const
{
    return m_2dBathymetry[i_y][i_x];
}

tsunami_lab::t_real tsunami_lab::setups::Checkpoint2d::getLastTimeStep() const {
    return m_lastTimeStep;
}