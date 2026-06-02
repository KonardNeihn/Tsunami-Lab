#pragma once

#include "../config/Config.h"
#include "../setups/Setup.h"
#include "../patches/WavePropagation.h"

namespace tsunami_lab {

void initialize(
    patches::WavePropagation* solver,
    setups::Setup* setup,
    const Config& config,
    t_real& hMax
);

}