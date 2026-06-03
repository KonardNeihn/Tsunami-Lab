#pragma once

#include <string>
#include "../config/Config.h"
#include "NetCdf.h"
#include "NetCdfCheckpoint.h"
#include "../patches/WavePropagation.h"

namespace tsunami_lab {

class OutputManager {
public:
    OutputManager(Config& g_config,
                  patches::WavePropagation* g_solver);

    void writeStep(t_real simTime);
    void writeCheckpoint(t_real simTime);
    void updateStations();

private:
    const Config& g_config;
    patches::WavePropagation* g_solver;

    std::string m_ncPath;
    io::NetCdfCheckpoint m_checkpoint;
    io::NetCdf m_ncWriter;

    t_idx m_lastCheckpointId = 2;

};

}