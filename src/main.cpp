/**
 * @author Alexander Breuer (alex.breuer AT uni-jena.de)
 *
 * @section DESCRIPTION
 * Entry-point for simulations.
 **/
#include "patches/WavePropagationAdaptiveGrid2d.h"
#include "patches/WavePropagation1d.h"
#include "patches/WavePropagation2d.h"
#include "io/Csv.h"
#include "io/NetCdf.h" 
#include "io/NetCdfCheckpoint.h"
#include "io/Station.h"
#include "io/XmlReader.h"
#include <vector>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <fstream>
#include <limits>
#include <filesystem>
#include "config/Config.h"
#include "factory/SetupFactory.h"
#include "core/Initializer.h"
#include "io/OutputManager.h"
#include <algorithm>
#include <chrono>

int main( int   i_argc,
          char *i_argv[] ) {
  std::vector<std::vector<tsunami_lab::t_idx>> l_gridResolution;
  tsunami_lab::patches::WavePropagation *l_waveProp;

  std::cout << "####################################" << std::endl;
  std::cout << "### Tsunami Lab                  ###" << std::endl;
  std::cout << "###                              ###" << std::endl;
  std::cout << "### https://scalable.uni-jena.de ###" << std::endl;
  std::cout << "####################################" << std::endl;

  // Handling input options
  Config g_config = parseArgs(i_argc, i_argv);

  // delete incomplete checkpoints
  if (std::filesystem::exists("solutions"))
  {
    for (const auto& entry : std::filesystem::directory_iterator("solutions")) {
      std::string name = entry.path().filename().string();

      if (entry.path().extension() == ".tmp") {
        std::filesystem::remove(entry.path());
      }
    } 
  }

  // detect last checkpoint
  if (g_config.setup == "Checkpoint2d") {
    std::vector<int> ids;

    for (const auto& entry : std::filesystem::directory_iterator("solutions"))
    {
        std::string name = entry.path().filename().string();

        if (name.substr(0, 10) == "checkpoint" && entry.path().extension() == ".nc")
        {
            std::string number =
                name.substr(
                    std::string("checkpoint").size(),
                    name.size()
                    - std::string("checkpoint").size()
                    - 3); // ".nc"

            ids.push_back(std::stoi(number));
        }
    }

    if (ids.empty()) {
        throw std::runtime_error("No checkpoint found");
    }
    std::sort(ids.begin(), ids.end());
    g_config.latestCheckpoint = ids.back();
  }


  // construct setup
  auto setup = createSetup(g_config);

  // current simulation time
  tsunami_lab::t_real l_simTime = g_config.simTime;

  // notify user about selectet configuration
  std::cout << "runtime configuration" << std::endl;
  std::cout << "  selected setup:                        " << g_config.setup << std::endl;
  std::cout << "  number of solver cells in x-direction: " << g_config.nx << std::endl;
  std::cout << "  number of solver cells in y-direction: " << g_config.ny << std::endl;
  std::cout << "  cell size in meters:                   " << g_config.dxy << std::endl;
  std::cout << "  starts at time (sec):                  " << l_simTime << std::endl;
  std::cout << "  ends at time (sec):                    " << g_config.endTime << std::endl;
  std::cout << "  selected solver:                       " << g_config.solver << std::endl;
  std::cout << "  insanity:                              " << g_config.insanity << std::endl;
  std::cout << "  coarseness:                            " << g_config.k << std::endl;


  //calculate actual number of cells in simulation
  tsunami_lab::t_real l_cellAmount = (g_config.nx * g_config.ny);

  // set output directory
  std::filesystem::path outDir = "solutions";

  // ensure output directory exists
  if (!std::filesystem::exists(outDir)) {
    std::filesystem::create_directory(outDir);
  } else {
    // delete old solution files
    for (const auto& entry : std::filesystem::directory_iterator(outDir)) {
      if (entry.is_regular_file()) {
        auto name = entry.path().filename().string();

        // only delete solution files
        if (name.rfind("solution_", 0) == 0 && entry.path().extension() == ".csv") {
          std::filesystem::remove(entry.path());
        }
      }
    }
  }

  // set output path for NetCdf
  std::string l_ncPath = "solutions/netcdf_output.nc";   

  // maximum observed height during setup
  tsunami_lab::t_real l_hMax = 0;

 // construct solver
 if( g_config.is_2d ) {
    l_waveProp =
      new tsunami_lab::patches::WavePropagationAdaptiveGrid2d(
          g_config.nx,
          g_config.ny
      );

    tsunami_lab::initializeAdaptiveGrid(
        l_waveProp,
        setup,
        g_config,
        l_hMax
      );

} else {
    l_waveProp =
      new tsunami_lab::patches::WavePropagation1d(
          g_config.nx,
          g_config.solver
      );

    tsunami_lab::initialize(
        l_waveProp,
        setup,
        g_config,
        l_hMax
    );
}

  tsunami_lab::determineGridResolution(l_hMax, 9, setup, g_config, l_gridResolution);

  // derive maximum wave speed in setup; the momentum is ignored
  tsunami_lab::t_real l_speedMax = std::sqrt( 9.81 * l_hMax );

  // derive constant time step; changes at simulation time are ignored
  tsunami_lab::t_real l_dt = 0.5 * g_config.dxy / l_speedMax;

  // derive scaling for a time step
  tsunami_lab::t_real l_scaling = l_dt / g_config.dxy;

  // set up time and print control
  tsunami_lab::t_idx  l_timeStep = 0;

  std::cout << "entering time loop" << std::endl;

  // Load stations from xml
  std::vector<tsunami_lab::io::StationConfig> stationConfigs;
  tsunami_lab::io::OutputConfig outputConfig;
  std::string stationsXmlPath = "src/io/stations.xml";
  tsunami_lab::io::loadXmlConfig(stationsXmlPath, stationConfigs, outputConfig);
  std::vector<tsunami_lab::io::Station> stations;
  if(g_config.is_2d) {
    for (const auto& stationConfig : stationConfigs) {
      // construct Object directly into vector
      stations.emplace_back(stationConfig.x,
                            stationConfig.y,
                            outputConfig.interval,
                            stationConfig.name,
                            g_config.nx,
                            outputConfig.path);
    }
  }

  tsunami_lab::OutputManager output(g_config, l_waveProp);

  if (g_config.setup != "Checkpoint2d") {
    output.deleteCheckpoints();
  }

  // time for entire loop
  auto l_loopStart = std::chrono::high_resolution_clock::now();

  // variable for solver time (added up)
  double l_solverTotalDuration = 0.0;

  // iterate over time
  while( l_simTime < g_config.endTime ) {
    if( l_timeStep % 25 == 0 ) {    
      std::cout << "  simulation time: " << l_simTime << "/" << g_config.endTime << " time steps: " << l_timeStep << std::endl;
      std::cout << "  writing wave field to " << l_ncPath << std::endl;

      // netCDF write
      output.writeStep(l_simTime);
      //if (g_config.is_2d) {       // checkpoints commented out for testing
      //  output.writeCheckpoint(l_simTime);
      //}
    }                                                                   

    // New setup for ghost-cells with false defaults in the setup.h
    l_waveProp->setGhostCells(
      setup->isLeftBoundaryReflecting(),
      setup->isRightBoundaryReflecting(),
      setup->isBottomBoundaryReflecting(),
      setup->isTopBoundaryReflecting()
    );

    // start seperate timer for only the solver
    auto l_solverStart = std::chrono::high_resolution_clock::now();
    
    l_waveProp->timeStep(l_scaling);
    
    auto l_solverEnd = std::chrono::high_resolution_clock::now();
    // calculate difference and then add it to the total
    std::chrono::duration<double> l_solverDiff = l_solverEnd - l_solverStart;
    l_solverTotalDuration += l_solverDiff.count();

    // Station Updates
    for (auto& station : stations) {
      // nullptr mit getMomentumY tasuchen sobald geht
      station.timeStep(l_dt, l_waveProp->getHeight(), l_waveProp->getMomentumX(), l_waveProp->getMomentumY());
    }

    l_timeStep++;
    l_simTime += l_dt;
  }
  //saving one last time
  std::cout << "  simulation time: " << l_simTime << " time steps: " << l_timeStep << std::endl;
  output.writeStep(l_simTime);

  std::cout << "finished time loop" << std::endl;

  // take time for the entire loop
  auto l_loopEnd = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> l_loopTotalDuration = l_loopEnd - l_loopStart;

  // write times to terminal
  std::cout << "Entire loop time " << l_loopTotalDuration.count() << " Seconds" << std::endl;
  std::cout << "Only Solver-time (l_waveProp): " << l_solverTotalDuration << " Seconds" << std::endl;
  if (l_loopTotalDuration.count() > 0) {
      std::cout << "Percentage of Solver-time for comparison: " 
                << (l_solverTotalDuration / l_loopTotalDuration.count()) * 100.0 << " %" << std::endl;
      std::cout << "Average time per cell in nanoseconds: " 
                << (l_solverTotalDuration / l_cellAmount ) * 1000000000 << " ns" << std::endl;
  }

  // free memory
  std::cout << "freeing memory" << std::endl;
  delete setup;
  delete l_waveProp;

  std::cout << "finished, exiting" << std::endl;
  return EXIT_SUCCESS;
}