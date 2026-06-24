#pragma once

#include "../config/Config.h"
#include "../setups/Setup.h"
#include "../patches/WavePropagation.h"
#include <vector>

namespace tsunami_lab {

void initialize(
    patches::WavePropagation* solver,
    setups::Setup* setup,
    const Config& g_config,
    t_real& o_hMax
);

void determineGridResolution(
    setups::Setup* setup,
    const Config& g_config,
    std::vector<std::vector<t_idx>>& o_gridResolution);
}