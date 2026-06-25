#include "Initializer.h"

namespace tsunami_lab {

void initialize(
    patches::WavePropagation* solver,
    setups::Setup* setup,
    const Config& g_config,
    t_real& o_hMax
) {

    tsunami_lab::patches::WavePropagationAdaptiveGrid2d* solver2 = dynamic_cast<tsunami_lab::patches::WavePropagationAdaptiveGrid2d*>(solver);

    // iterating over ny cells
    for (t_idx l_solverCellY = 0; l_solverCellY < g_config.ny; l_solverCellY++) {
        // iterating over nx cells
        for (t_idx l_solverCellX = 0; l_solverCellX < g_config.nx; l_solverCellX++) {

            t_real l_h  = setup->getHeight(l_solverCellX, l_solverCellY);
            t_real l_hu = setup->getMomentumX(l_solverCellX, l_solverCellY);
            t_real l_hv = setup->getMomentumY(l_solverCellX, l_solverCellY);
            t_real l_b  = setup->getBathymetry(l_solverCellX, l_solverCellY);

            o_hMax = std::max(o_hMax, l_h);

            if (solver2) {
                solver2->setHeight(l_solverCellX, l_solverCellY, l_h, setup);
                solver2->setMomentumX(l_solverCellX, l_solverCellY, l_hu);
                solver2->setMomentumY(l_solverCellX, l_solverCellY, l_hv);
                solver2->setBathymetry(l_solverCellX, l_solverCellY, l_b, setup);
                continue;
            }

            solver->setHeight(l_solverCellX, l_solverCellY, l_h);
            solver->setMomentumX(l_solverCellX, l_solverCellY, l_hu);
            solver->setMomentumY(l_solverCellX, l_solverCellY, l_hv);
            solver->setBathymetry(l_solverCellX, l_solverCellY, l_b);
        }
    }
}

// Funktion, um die Gitterauflösung basierend auf Wassertiefe zu bestimmen
void determineGridResolution(
    t_real i_hMax,  // wie viel eine Zelle maximal unterteilt werden kann
    t_idx i_resMax, // die maximale wassertiefe
    setups::Setup* setup,
    const Config& g_config,
    std::vector<std::vector<t_idx>>& o_gridResolution  // Ausgabe: gewünschte vergrößerung pro Zelle
    ) {

    o_gridResolution.resize(g_config.ny, std::vector<t_idx>(g_config.nx, 0)); // Initialisierung

    // iterating over ny cells
    for (t_idx l_solverCellY = 0; l_solverCellY < g_config.ny; l_solverCellY++) {
        // iterating over nx cells
        for (t_idx l_solverCellX = 0; l_solverCellX < g_config.nx; l_solverCellX++) {
            t_real l_h  = setup->getHeight(l_solverCellX, l_solverCellY);
            t_idx resolution;
            resolution = i_resMax - ((l_h - 1) * ((i_resMax - 1) / i_hMax)); // absolute crazy rechnung. stimmt aber

            o_gridResolution[l_solverCellY][l_solverCellX] = resolution;
            std::cout << resolution;
        }
        std::cout << std::endl;
    }
}

}