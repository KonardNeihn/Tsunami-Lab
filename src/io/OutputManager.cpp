#include "OutputManager.h"
#include <iostream>

namespace tsunami_lab {

OutputManager::OutputManager(const Config& config,
                             patches::WavePropagation* solver)
: g_config(config),
  g_solver(solver),
  // set output path for NetCdf
  m_ncPath("solutions/netcdf_output.nc"),
  // construct writer for NetCdf
  m_ncWriter(
      m_ncPath,
      config.nx,
      config.is_2d ? config.ny : 1,
      config.dxy,
      solver->getStride(),
      solver->getBathymetry()
  ),
  m_checkpoint(config),
  m_checkpointTimer(0.0)
{}

void OutputManager::writeStep()
{
    m_ncWriter.write(
        g_config.simTime,
        g_solver->getHeight(),
        g_solver->getMomentumX(),
        g_config.is_2d ? g_solver->getMomentumY() : nullptr
    );
}

void OutputManager::writeCheckpointIfNeeded(t_real dt)
{
    m_checkpointTimer += dt;

    if (m_checkpointTimer < 0.5) return;

    std::string path = "solutions/checkpoint.nc";

    std::cout << "Creating checkpoint at " << path << std::endl;

    m_checkpoint.createCheckpoint(
        path,
        g_config.nx,
        g_config.ny,
        g_config.simTime,
        g_config.endTime,
        g_config.width,
        0.0,
        0.0
    );

    m_checkpoint.write2DVariable(path, "height",
        g_solver->getHeight(), g_config.nx, g_config.ny);

    m_checkpoint.write2DVariable(path, "bathymetry",
        g_solver->getBathymetry(), g_config.nx, g_config.ny);

    m_checkpoint.write2DVariable(path, "momentumX",
        g_solver->getMomentumX(), g_config.nx, g_config.ny);

    if (g_config.is_2d) {
        m_checkpoint.write2DVariable(path, "momentumY",
            g_solver->getMomentumY(), g_config.nx, g_config.ny);
    }

    std::cout << "Checkpoint finished\n";

    m_checkpointTimer = 0.0;
}

}