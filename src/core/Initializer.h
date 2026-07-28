#pragma once

#include "../config/Config.h"
#include "../setups/Setup.h"
#include "../patches/WavePropagation.h"
#include "../patches/WavePropagationAdaptiveGrid2d.h"
#include <vector>
#include <iostream>

namespace tsunami_lab {

/*
* initializes the selected setup for a wavepropagation
*/
void initialize(
    patches::WavePropagation* solver,
    setups::Setup* setup,
    const Config& g_config,
    t_real& o_hMax
);

/*
* helps initialize the wavepropagation with adaptive grids
*/
void initializeAdaptiveGrid(
    patches::WavePropagation* solver,
    setups::Setup* setup,
    const Config& g_config,
    t_real& o_hMax,
    std::vector<std::vector<t_idx>>& i_gridResolution
);

/*
* Determines a grid with different sized subcells
*/ 
void determineGridResolution(
    setups::Setup* setup,
    const Config& g_config,
    std::vector<std::vector<t_idx>>& o_gridResolution);
}