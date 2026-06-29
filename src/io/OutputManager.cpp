#include "OutputManager.h"
#include <iostream>
#include <filesystem>
#include <algorithm>

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
                g_config.k,                  // currently disabled  
                g_config.isCheckpoint
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
    std::string tmpPath = "solutions/checkpoint" + std::to_string(g_config.latestCheckpoint + 1) + ".nc.tmp";
    std::string finalPath = "solutions/checkpoint" + std::to_string(g_config.latestCheckpoint + 1) + ".nc";

    std::cout << "Creating checkpoint at " << tmpPath << std::endl;

    m_checkpoint.createCheckpoint(tmpPath, simTime);

    m_checkpoint.write2DVariable(   tmpPath, 
                                    "height",
                                    g_solver->getHeight(), 
                                    g_config.nx, 
                                    g_config.ny);

    m_checkpoint.write2DVariable(   tmpPath, 
                                    "bathymetry",
                                    g_solver->getBathymetry(), 
                                    g_config.nx, 
                                    g_config.ny);

    m_checkpoint.write2DVariable(   tmpPath, 
                                    "momentumX",
                                    g_solver->getMomentumX(), 
                                    g_config.nx, 
                                    g_config.ny);

    if (g_config.is_2d) {
        m_checkpoint.write2DVariable(   tmpPath, 
                                        "momentumY",
                                        g_solver->getMomentumY(), 
                                        g_config.nx, 
                                        g_config.ny);
    }

    std::cout << "Finished creating Checkpoint" << std::endl;

    std::filesystem::rename(tmpPath, finalPath);

    g_config.latestCheckpoint++;
}

void OutputManager::deleteCheckpoints()
{
    for (const auto& entry : std::filesystem::directory_iterator("solutions")) {
        std::string name = entry.path().filename().string();

        if (name.substr(0, 10) == "checkpoint") {
            std::filesystem::remove(entry.path());
        }
    }
}

void OutputManager::detectLatestCheckpoint()
{
    deleteUncompletedCheckpoints();
    std::vector<int> ids;

    for (const auto& entry : std::filesystem::directory_iterator("solutions"))
    {
        std::string name = entry.path().filename().string();

        if (name.substr(0, 10) == "checkpoint" && entry.path().extension() == ".nc")
        {
            std::string number =
                name.substr(
                    std::string("checkpoint").size(),
                    name.size()
                    - std::string("checkpoint").size()
                    - 3); // ".nc"

            ids.push_back(std::stoi(number));
        }
    }

    if (ids.empty()) {
        throw std::runtime_error("No checkpoint found");
    }
    std::sort(ids.begin(), ids.end());
    g_config.latestCheckpoint = ids.back();
}

void OutputManager::deleteUncompletedCheckpoints()
{

    namespace fs = std::filesystem;

    for (const auto& entry : std::filesystem::directory_iterator("solutions"))
    {
        if (entry.path().extension() == ".tmp")
        {
            std::cout << "Removing incomplete checkpoint " << entry.path() << std::endl;
            std::filesystem::remove(entry.path());
        }
    }
}

}