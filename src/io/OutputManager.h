#pragma once

#include <string>
#include "../config/Config.h"
#include "NetCdf.h"
#include "NetCdfCheckpoint.h"
#include "../patches/WavePropagation.h"

namespace tsunami_lab {

class OutputManager {
public:
    OutputManager(const Config& g_config,
                  patches::WavePropagation* g_solver);

    void writeStep();
    void writeCheckpointIfNeeded(t_real dt);
    void updateStations();

private:
    const Config& g_config;
    patches::WavePropagation* g_solver;

    std::string m_ncPath;
    io::NetCdf m_ncWriter;
    io::NetCdfCheckpoint m_checkpoint;

    t_real m_checkpointTimer;
};

}