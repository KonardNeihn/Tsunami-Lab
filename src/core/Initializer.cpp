#include "Initializer.h"

namespace tsunami_lab {

void initialize(
    patches::WavePropagation* solver,
    setups::Setup* setup,
    const Config& config,
    t_real& hMax
) {
    // iterating over ny cells
    for (t_idx l_cy = 0; l_cy < config.ny; l_cy++) {
        // iterating over nx cells
        for (t_idx l_cx = 0; l_cx < config.nx; l_cx++) {

            //calculating corresponding domain index l_x and l_y is in meters
            t_real l_x = (l_cx + 0.5) * config.dxy + config.domainStartX;
            t_real l_y = config.is_2d
                ? (l_cy + 0.5) * config.dxy + config.domainStartY
                : l_cy * config.dxy;

            t_real l_h  = setup->getHeight(l_x, l_y);
            t_real l_hu = setup->getMomentumX(l_x, l_y);
            t_real l_hv = setup->getMomentumY(l_x, l_y);
            t_real l_b  = setup->getBathymetry(l_x, l_y);

            hMax = std::max(hMax, l_h);

            solver->setHeight(l_cx, l_cy, l_h);
            solver->setMomentumX(l_cx, l_cy, l_hu);
            solver->setMomentumY(l_cx, l_cy, l_hv);
            solver->setBathymetry(l_cx, l_cy, l_b);
        }
    }
}

}