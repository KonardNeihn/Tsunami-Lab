/**
 * @author 
 *
 * @section DESCRIPTION
 * 2d Checkpoint setup.
 **/
#include "Checkpoint2d.h"

tsunami_lab::setups::Checkpoint2d::Checkpoint2d(
   Config& config)
: g_config(config)
{
    m_checkpointPath = "solutions/checkpoint" + std::to_string(g_config.latestCheckpoint) + ".nc";

    tsunami_lab::io::NetCdfCheckpoint l_checkpointReader(g_config);
    
    l_checkpointReader.readCheckpointAndSetParameters( m_checkpointPath );

    m_2dBathymetry = l_checkpointReader.read2DVariable(m_checkpointPath, "bathymetry");
    m_2dDisplacement = l_checkpointReader.read2DVariable(m_checkpointPath, "height");
    m_2dMomentumX = l_checkpointReader.read2DVariable(m_checkpointPath, "momentumX");
    m_2dMomentumY = l_checkpointReader.read2DVariable(m_checkpointPath, "momentumY");
}

tsunami_lab::t_real tsunami_lab::setups::Checkpoint2d::getHeight(t_real solverX, t_real solverY) const
{
    return m_2dDisplacement[solverY][solverX];
}

tsunami_lab::t_real tsunami_lab::setups::Checkpoint2d::getMomentumX(t_real solverX, t_real solverY) const {
  return m_2dMomentumX[solverY][solverX];
}

tsunami_lab::t_real tsunami_lab::setups::Checkpoint2d::getMomentumY(t_real solverX, t_real solverY) const {
  return m_2dMomentumY[solverY][solverX];
}

tsunami_lab::t_real tsunami_lab::setups::Checkpoint2d::getBathymetry(t_real solverX, t_real solverY) const
{
    return m_2dBathymetry[solverY][solverX];
}

tsunami_lab::t_real tsunami_lab::setups::Checkpoint2d::getLastTimeStep() const {
    return m_lastTimeStep;
}