#include "Initializer.h"

namespace tsunami_lab {

void initialize(
    patches::WavePropagation* solver,
    setups::Setup* setup,
    const Config& g_config,
    t_real& o_hMax
) {
    // iterating over ny cells
    for (t_idx l_solverCellY = 0; l_solverCellY < g_config.ny; l_solverCellY++) {
        // iterating over nx cells
        for (t_idx l_solverCellX = 0; l_solverCellX < g_config.nx; l_solverCellX++) {

            t_real l_h  = setup->getHeight(l_solverCellX, l_solverCellY);
            t_real l_hu = setup->getMomentumX(l_solverCellX, l_solverCellY);
            t_real l_hv = setup->getMomentumY(l_solverCellX, l_solverCellY);
            t_real l_b  = setup->getBathymetry(l_solverCellX, l_solverCellY);

            o_hMax = std::max(o_hMax, l_h);

            solver->setHeight(l_solverCellX, l_solverCellY, l_h);
            solver->setMomentumX(l_solverCellX, l_solverCellY, l_hu);
            solver->setMomentumY(l_solverCellX, l_solverCellY, l_hv);
            solver->setBathymetry(l_solverCellX, l_solverCellY, l_b);
        }
    }
}

}