#include "Initializer.h"
#include <cmath>      // std::round
#include <algorithm>  // std::clamp
#include <iomanip>  // std::fixed und setprecision

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
    if (solver2) {
        solver2->syncCoarseBathymetry();
    }
}

// Funktion, um die Gitterauflösung basierend auf Wassertiefe zu bestimmen
void determineGridResolution(
    setups::Setup* setup,
    const Config& g_config,
    std::vector<std::vector<t_idx>>& o_gridResolution
    ) {

    const t_real SHELF_DEPTH = 1000.0;  // flacher -> Aufloesung 2
    const t_real COAST_DEPTH =  200.0;  // flacher -> Aufloesung 4 (Land h==0 inklusive)

    // so there are just 2^x values
    const t_idx maxRes = t_idx(1) << (g_config.maximalCellResolution - 1);
    const t_idx nx = g_config.nx;
    const t_idx ny = g_config.ny;

    // Initialisierung
    o_gridResolution.assign(ny, std::vector<t_idx>(nx, 1));

    // iterating over ny cells
    for (t_idx y = 0; y < ny; y++) {
        // iterating over nx cells
        for (t_idx x = 0; x < nx; x++) {
            t_real l_h = setup->getHeight(x, y);

            t_idx resolution = 1;
            if (l_h < SHELF_DEPTH) resolution = 2;
            if (l_h < COAST_DEPTH) resolution = 4;

            // just to be sure its in the right range
            o_gridResolution[y][x] = std::min(resolution, maxRes);
        }
    }

    // --- nur die groesste zusammenhaengende verfeinerte Region behalten ---
    {
        std::vector<int>   comp(nx * ny, -1);
        std::vector<t_idx> compSize;
        int nComp = 0;

        for (t_idx y0 = 0; y0 < ny; y0++) {
            for (t_idx x0 = 0; x0 < nx; x0++) {
                if (o_gridResolution[y0][x0] <= 1) continue;
                if (comp[y0 * nx + x0] != -1) continue;

                std::vector<std::pair<t_idx, t_idx>> stack;
                stack.push_back({x0, y0});
                comp[y0 * nx + x0] = nComp;
                t_idx size = 0;

                while (!stack.empty()) {
                    auto [cx, cy] = stack.back();
                    stack.pop_back();
                    size++;

                    const long ox[4] = {1, -1, 0, 0};
                    const long oy[4] = {0, 0, 1, -1};
                    for (int k = 0; k < 4; k++) {
                        long ux = static_cast<long>(cx) + ox[k];
                        long uy = static_cast<long>(cy) + oy[k];
                        if (ux < 0 || uy < 0) continue;
                        if (ux >= static_cast<long>(nx)) continue;
                        if (uy >= static_cast<long>(ny)) continue;

                        t_idx tx = static_cast<t_idx>(ux);
                        t_idx ty = static_cast<t_idx>(uy);
                        if (o_gridResolution[ty][tx] <= 1) continue;
                        if (comp[ty * nx + tx] != -1) continue;

                        comp[ty * nx + tx] = nComp;
                        stack.push_back({tx, ty});
                    }
                }

                compSize.push_back(size);
                nComp++;
            }
        }

        if (nComp > 0) {
            int best = 0;
            for (int c = 1; c < nComp; c++) {
                if (compSize[c] > compSize[best]) best = c;
            }

            t_idx dropped = 0;
            for (t_idx y = 0; y < ny; y++) {
                for (t_idx x = 0; x < nx; x++) {
                    if (comp[y * nx + x] != -1 && comp[y * nx + x] != best) {
                        o_gridResolution[y][x] = 1;
                        dropped++;
                    }
                }
            }
        }
    }

    std::map<t_idx, t_idx> cnt;
    for (t_idx y = 0; y < ny; y++) {
        for (t_idx x = 0; x < nx; x++) {
            cnt[o_gridResolution[y][x]]++;
        }
    }
}

}