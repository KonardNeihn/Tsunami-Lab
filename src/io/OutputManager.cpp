#include "OutputManager.h"
#include <iostream>

namespace tsunami_lab {

OutputManager::OutputManager(Config& config,
                             patches::WavePropagation* solver)
: g_config(config),
  g_solver(solver),
  m_ncPath("solutions/netcdf_output.nc"),
  m_checkpoint(config),
  m_ncWriter( m_ncPath,                                         
                g_config.nx,                                             
                g_config.is_2d ? g_config.ny : 1,          // pass 1 for 1D runs
                g_config.dxy,                                            
                g_solver->getStride(),                          
                g_solver->getBathymetry(), 
                g_config.k                  // currently disabled               
  )   
{}

void OutputManager::writeStep(t_real simTime)
{
    m_ncWriter.write(   simTime,
                        g_solver->getHeight(),
                        g_solver->getMomentumX(),
                        g_config.is_2d ? g_solver->getMomentumY() : nullptr );
}

void OutputManager::writeCheckpoint(t_real simTime)
{   
    t_idx l_currentCheckpointId = 1;
    if (m_lastCheckpointId == 1) {
        l_currentCheckpointId = 2;
    }
    std::string path = "solutions/checkpoint" + std::to_string(l_currentCheckpointId) + ".nc";

    std::cout << "Creating checkpoint at " << path << std::endl;

    m_checkpoint.createCheckpoint(path, simTime);

    m_checkpoint.write2DVariable(   path, 
                                    "height",
                                    g_solver->getHeight(), 
                                    g_config.nx, 
                                    g_config.ny);

    m_checkpoint.write2DVariable(   path, 
                                    "bathymetry",
                                    g_solver->getBathymetry(), 
                                    g_config.nx, 
                                    g_config.ny);

    m_checkpoint.write2DVariable(   path, 
                                    "momentumX",
                                    g_solver->getMomentumX(), 
                                    g_config.nx, 
                                    g_config.ny);

    if (g_config.is_2d) {
        m_checkpoint.write2DVariable(   path, 
                                        "momentumY",
                                        g_solver->getMomentumY(), 
                                        g_config.nx, 
                                        g_config.ny);
    }

    std::cout << "Finished creating Checkpoint" << std::endl;

    m_lastCheckpointId = l_currentCheckpointId;

}

}