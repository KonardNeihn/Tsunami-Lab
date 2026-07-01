#pragma once

#include <string>
#include "../config/Config.h"
#include "NetCdfAdaptiveGrid.h"
#include "NetCdfCheckpoint.h"
#include "../patches/WavePropagation.h"

namespace tsunami_lab {

class OutputManager {
public:
    OutputManager(  Config& g_config,
                    patches::WavePropagation* g_solver,
                    const std::vector<std::vector<tsunami_lab::t_idx>>& gridResolutionLevels);

    void writeStep(t_real simTime);
    void writeCheckpoint(t_real simTime);
    void updateStations();
    void deleteCheckpoints();
    void detectLatestCheckpoint();
    void deleteUncompletedCheckpoints();

private:
    Config& g_config;
    patches::WavePropagation* g_solver;

    std::string m_ncPath;
    io::NetCdfCheckpoint m_checkpoint;
    io::NetCdfAdaptiveGrid m_ncWriter;

    std::vector<std::vector<tsunami_lab::t_idx>> m_gridResolutionLevels;

    // t_idx k = 4  // currently disabled

};

}