#include "Initializer.h"
#include "../patches/WavePropagationAdaptiveGrid2d.h"
#include "../setups/Setup.h"
#include "../config/Config.h"
#include <map>

void tsunami_lab::initializeAdaptiveGrid(
    patches::WavePropagation* solver,
    setups::Setup* setup,
    const Config& g_config,
    [[maybe_unused]] t_real& o_hMax) {
  
  // Query setup for resolution at each cell
  std::map<std::pair<t_idx, t_idx>, t_idx> refinementMap;
  
  for (t_idx iy = 0; iy < g_config.ny; iy++) {
    for (t_idx ix = 0; ix < g_config.nx; ix++) {
      // Convert grid index to physical coordinates
      // Assuming domain starts at (0, 0)
      t_real x = ix * g_config.dxy;
      t_real y = iy * g_config.dxy;
      
      // Query setup for resolution at this location
      t_idx resolution = setup->getResolution(x, y);
      refinementMap[{ix, iy}] = resolution;
    }
  }
  
  // Set the refinement map in the adaptive solver
  auto adaptiveGrid = dynamic_cast<patches::WavePropagationAdaptiveGrid2d*>(solver);
  if (adaptiveGrid) {
    adaptiveGrid->setRefinementMap(refinementMap);
  }
  
  // Now do normal initialization (sets heights, momentums, bathymetry)
  tsunami_lab::initialize(solver, setup, g_config, o_hMax);
}